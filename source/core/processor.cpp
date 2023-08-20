#include <core/processor.hpp>
#include <core/machine.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>
#include <win32/scope_name.hpp>

#include <utils/capstone.hpp>

#include <utils/literals.hpp>
#include <utils/lambda.hpp>
#include <utils/logger.hpp>
#include <utils/span.hpp>

#include <future>

using namespace size_literals;

using core::Processor;

Processor::Processor(Machine& machine_v, std::uint32_t vcpuindex_v)
	: WHvProcessor{ machine_v.GetPartition(), vcpuindex_v }
	, m_Machine{ machine_v }
	, m_Suspend{ 0u }
{}

Processor::~Processor()
{ 
	m_Stopper.request_stop();
	m_Suspend.release();
}

auto Processor::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t
{
	return m_Machine.IoPortAccess(is_write_v, port_v, data_v);
}

auto Processor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t
{	
	std::int32_t result_v{ ERROR_SUCCESS };
	result_v = WHvProcessor::MemoryAccess(is_write_v, physaddr_v, data_v);
	if (SUCCEEDED(result_v))
		return result_v;
	result_v = m_Machine.MemoryAccess(is_write_v, physaddr_v, data_v);		
	if (SUCCEEDED(result_v))
		return result_v;
	return result_v;
}

auto Processor::GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t
{
	return WHvProcessor::GetRegisters(names_v, values_v);
}

auto Processor::SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t
{
	return WHvProcessor::SetRegisters(names_v, values_v);
}

auto Processor::TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_o, std::uint64_t& addr_o) const -> std::int32_t
{
	auto const [status_v, code_v, addr_v] = WHvProcessor::TranslateGva(virtaddr_v, flags_v);
	addr_o = addr_v; code_o = code_v; return status_v;
}

auto Processor::TestIRQDeliverability() -> std::int32_t {
	std::shared_lock lock_v{ m_IRQPendingLock };
	auto irq_pending_v = m_IRQPendingMask.any();
	auto nmi_pending_v = m_NmiPending;
	if (irq_pending_v || nmi_pending_v) {
		return SetRegister(WHvX64RegisterDeliverabilityNotifications, { .DeliverabilityNotifications = {
			.NmiNotification = nmi_pending_v ? 1u : 0u, .InterruptNotification = irq_pending_v ? 1u : 0u
		}});
	}
	return ERROR_SUCCESS;
}

auto Processor::PopPendingIRQ() -> std::optional<std::uint8_t>
{
	std::unique_lock lock_v{ m_IRQPendingLock };
	std::uint16_t vector_v{ 0u };
	while (vector_v < 256u) {
		if (m_IRQPendingMask.test(vector_v)) {
			m_IRQPendingMask.reset(vector_v);
			return vector_v & 0xFFu;
		}
		vector_v += 1u;
	}
	return std::nullopt;
}

auto Processor::PopPendingNmi() -> std::optional<std::uint8_t>
{
	std::unique_lock lock_v{ m_IRQPendingLock };
	if (!m_NmiPending) return std::nullopt;
	m_NmiPending = false;
	return 2u;
}

auto Processor::Run(std::stop_token stoppee_v) -> exit_result_type
{
	win32::scope_name _tdesc{ "Processor::Run" };
	using utils::logger;
	std::stop_callback stopcbk_v{ stoppee_v, [this] { 
		WHvProcessor::Cancel();
		m_Suspend.release();
	}};

	std::unique_lock lock_v{ m_IsRunning };
	auto& emulator_v = Emulator();
	while (!stoppee_v.stop_requested())
	{		
		auto const result_v = WHvProcessor::Run();
		auto [status_v, context_v] = result_v;
		if (FAILED(status_v)) return result_v;		
		status_v = TestIRQDeliverability();
		if (FAILED(status_v)) return { status_v, context_v };
		switch (context_v.ExitReason)
		{
		case WHvRunVpExitReasonX64MsrAccess:
			if (context_v.MsrAccess.AccessInfo.IsWrite) {
				logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) write at {:#06x}:{:#010x}, EDX:EAX={:010X}:{:010X}", 
					GetIndex(), context_v.MsrAccess.MsrNumber,context_v.VpContext.Cs.Selector, context_v.VpContext.Rip,
					context_v.MsrAccess.Rdx, context_v.MsrAccess.Rax);
			} else {
				logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) read at {:#06x}:{:#010x}",
					GetIndex(), context_v.MsrAccess.MsrNumber, context_v.VpContext.Cs.Selector, context_v.VpContext.Rip);
			}
			status_v = SetRegister(WHvX64RegisterRip, { .Reg64 = context_v.VpContext.Rip + context_v.VpContext.InstructionLength });			
			if (FAILED(status_v)) return { status_v, context_v };			
			continue;
		case WHvRunVpExitReasonX64IoPortAccess:
			emulator_v.TryIoEmulation(*this, context_v.VpContext, context_v.IoPortAccess);
			continue;
		case WHvRunVpExitReasonMemoryAccess:
			emulator_v.TryMmioEmulation(*this, context_v.VpContext, context_v.MemoryAccess);
			continue;		
		case WHvRunVpExitReasonX64InterruptWindow:			
			switch (context_v.InterruptWindow.DeliverableType)
			{ 
			case WHvX64PendingInterrupt:
				{
					auto vector_o = PopPendingIRQ();
					if (!vector_o.has_value()) break;				
					status_v = SetRegister(WHvRegisterPendingEvent, { .ExtIntEvent = {
						.EventPending = 1u,
						.EventType = WHvX64PendingEventExtInt,
						.Vector = vector_o.value()
					}});
					if (FAILED(status_v)) {
						logger::error(logger::deflog, "CPU[{}] failed to inject interrupt: {:#010x}", GetIndex(), status_v);
						return { status_v, context_v };
					}
				}
				break;
			case WHvX64PendingNmi:
				{
					auto vector_o = PopPendingNmi();
					if (!vector_o.has_value()) break;
					status_v = SetRegister(WHvRegisterPendingEvent, { .ExtIntEvent = {
						.EventPending = 1u,
						.EventType = WHvX64PendingNmi,
						.Vector = vector_o.value()
					}});
					if (FAILED(status_v)) {
						logger::error(logger::deflog, "CPU[{}] failed to inject NMI: {:#010x}", GetIndex(), status_v);
						return { status_v, context_v };
					}
				}
				break;
			default: 
				break;
			}
			continue;
		case WHvRunVpExitReasonX64Halt:
			if (InterruptsEnabled()) { 
				m_Suspend.acquire();
				continue;
			}			
			[[fallthrough]];
		case WHvRunVpExitReasonCanceled:
			return result_v;
		case WHvRunVpExitReasonException:
			logger::error(logger::deflog, "CPU[{}] raised exception: {:d}({:#04X}) at {:04X}:{:08X}.", 
				GetIndex(), context_v.VpException.ExceptionType,
				context_v.VpException.ExceptionType,
				context_v.VpContext.Cs.Selector,
				context_v.VpContext.Rip				
			);
			[[fallthrough]];		
		default:
			return result_v;
		}
	}
	// In case the cpu was suspended, 
	// eat the injected cacel event
	return WHvProcessor::Run();
}

auto Processor::RunAsync() -> exit_future_type
{
	std::unique_lock lock_v{ m_IsRunning, std::try_to_lock };
	if (!lock_v.owns_lock()) {
		if (m_FutureExit.valid())
			return m_FutureExit;
		throw std::runtime_error(
			"Processor already running, but no future available"
		);
	}
	m_Stopper = std::stop_source{};
	m_FutureExit = std::async(std::launch::async, utils::lambda(this, &Processor::Run),
		m_Stopper.get_token()).share();
	return m_FutureExit;
}

auto Processor::CancelAsync() -> void {
	std::unique_lock lock_v{ m_IsRunning, std::try_to_lock };
	if (lock_v.owns_lock()) return;	
	m_Stopper.request_stop();
	lock_v.lock();
}

auto Processor::InjectInterrupt(std::uint8_t vector_v) -> std::int32_t
{	
	std::unique_lock lock_v{ m_IRQPendingLock };
	m_IRQPendingMask.set(vector_v, true);
	Unsuspend();
	return S_OK;
}

auto Processor::InjectNonMaskable() -> std::int32_t
{
	std::unique_lock lock_v{ m_IRQPendingLock };
	m_NmiPending = true;
	Unsuspend();
	return S_OK;
}

auto Processor::Unsuspend()  -> void
{
	m_Suspend.release();
}

auto Processor::Emulator() -> win32::WHvEmulator&
{
	using win32::WHvEmulator;
	static WHvEmulator emulator_v{ WHvEmulator::Create() };
  return emulator_v;
}

auto Processor::InterruptsEnabled() const -> bool
{
	WHV_REGISTER_VALUE rflags_v{};
	WIN32_ERROR_ASSERT(GetRegister(WHvX64RegisterRflags, rflags_v));
	return !!(rflags_v.Reg64 & kInterruptFlag);
}

auto Processor::AdvanceInstruction(WHV_VP_EXIT_CONTEXT const& vpcontext_v) const -> std::int32_t
{
	return SetRegister(WHvX64RegisterRip, { 
		.Reg64 = vpcontext_v.InstructionLength
		       + vpcontext_v.Rip
	});
}
