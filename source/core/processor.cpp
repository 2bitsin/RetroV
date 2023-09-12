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
	return m_Machine.IoPortAccess(*this, is_write_v, port_v, data_v);
}

auto Processor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v) const -> std::int32_t
{	
	std::int32_t result_v{ ERROR_SUCCESS };
	result_v = WHvProcessor::MemoryAccess(is_write_v, physaddr_v, data_v);
	if (ERROR_SUCCESS==result_v)
		return result_v;
	result_v = m_Machine.MemoryAccess(*this, is_write_v, physaddr_v, data_v);		
	if (ERROR_SUCCESS==result_v)
		return result_v;
	__debugbreak();
	return ERROR_ACCESS_DENIED;
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
	std::int32_t status_v{ S_OK };
	std::tie(status_v, code_o, addr_o) = WHvProcessor::TranslateGva(virtaddr_v, flags_v);	
	return status_v;
}

auto Processor::UnhandledMsr(WHV_VP_EXIT_CONTEXT const& context_v, WHV_X64_MSR_ACCESS_CONTEXT const& access_v) -> std::int32_t
{
	using utils::logger;
	if (access_v.AccessInfo.IsWrite) {
		logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) write at {:#06x}:{:#010x}, EDX:EAX={:010X}:{:010X}",GetIndex(), access_v.MsrNumber, context_v.Cs.Selector, context_v.Rip, access_v.Rdx, access_v.Rax);
	} else {
		logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) read at {:#06x}:{:#010x}", GetIndex(), access_v.MsrNumber, context_v.Cs.Selector, context_v.Rip);
	}
	return S_OK;
}

auto Processor::UnhandledException(WHV_VP_EXIT_CONTEXT const& context_v, WHV_VP_EXCEPTION_CONTEXT const& exception_v) -> std::int32_t
{
	using utils::logger;
	logger::error(logger::deflog, "CPU[{}] raised exception: {:d}({:#04X}) at {:04X}:{:08X}.", GetIndex(), exception_v.ExceptionType, exception_v.ExceptionType, context_v.Cs.Selector, context_v.Rip);  
	return S_OK;
}

auto Processor::RunToExit(std::stop_token stoppee_v) -> exit_result_type
{
	win32::scope_name _tdesc{ "Processor::RunToExit" };
	using utils::logger;
	std::stop_callback stopcbk_v{ stoppee_v, [this] { 
		WHvProcessor::Cancel();
		m_Suspend.release();
	}};
	
	std::unique_lock lock_v{ m_IsRunning };
	auto& emulator_v = Emulator();
	while (!stoppee_v.stop_requested())
	{		
		auto const result_v = WHvProcessor::RunToExit();
		auto [status_v, context_v] = result_v;
		if (ERROR_SUCCESS != status_v) 
			return result_v;	
		switch (context_v.ExitReason)
		{
		case WHvRunVpExitReasonX64IoPortAccess:
			emulator_v.TryIoEmulation(*this, context_v.VpContext, context_v.IoPortAccess);
			continue;
		case WHvRunVpExitReasonMemoryAccess:
			emulator_v.TryMmioEmulation(*this, context_v.VpContext, context_v.MemoryAccess);
			continue;		
		case WHvRunVpExitReasonHypercall:
			m_Machine.Hypercall(*this, context_v.VpContext, context_v.Hypercall);
			AdvanceInstruction(context_v.VpContext);
			continue;

		case WHvRunVpExitReasonSynicSintDeliverable:
			__debugbreak();
			continue;			
		case WHvRunVpExitReasonX64InterruptWindow:
			__debugbreak();
			continue;

		case WHvRunVpExitReasonX64Halt:
			if (InterruptsEnabled()) { 
				m_Suspend.acquire();
				continue; }			
			return result_v;
		case WHvRunVpExitReasonCanceled:
			if (stoppee_v.stop_requested())
				return result_v;
			m_Suspend.acquire();
			continue;

		case WHvRunVpExitReasonX64MsrAccess:
			status_v = UnhandledMsr(context_v.VpContext, context_v.MsrAccess);
			if (ERROR_SUCCESS != status_v)
				return { status_v, context_v };
			status_v = AdvanceInstruction(context_v.VpContext);
			if (ERROR_SUCCESS != status_v)
				return { status_v, context_v };
			continue;
		case WHvRunVpExitReasonException:
			status_v = UnhandledException(context_v.VpContext, context_v.VpException);
			if (ERROR_SUCCESS != status_v)
				return { status_v, context_v };
			[[fallthrough]];		
		default:
			return result_v;
		}
	}
	// In case the cpu was suspended, 
	// eat the injected cacel event
	return WHvProcessor::RunToExit();
}

auto Processor::Start() -> exit_future_type
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
	m_FutureExit = std::async(std::launch::async, utils::lambda(this, &Processor::RunToExit),
		m_Stopper.get_token()).share();
	return m_FutureExit;
}

auto Processor::Stop() -> void {
	std::unique_lock lock_v{ m_IsRunning, std::try_to_lock };
	if (lock_v.owns_lock()) return;	
	m_Stopper.request_stop();
	lock_v.lock();
}

auto Processor::Suspend() -> void
{
	std::unique_lock lock_v{ m_IsRunning, std::try_to_lock };
	if (lock_v.owns_lock()) return;
	WHvProcessor::Cancel();
}

auto Processor::Resume()  -> void
{
	m_Suspend.release();
}

auto Processor::ReadTsc() const -> std::tuple<std::int32_t, std::uint64_t>
{
	WHV_REGISTER_VALUE value_v{};
	auto result_v = GetRegister(WHvRegisterReferenceTsc, value_v);
	return { result_v, value_v.Reg64 };
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

auto Processor::SetSingleStepMode(bool is_debug_v) -> void
{
	auto flags_v = GetRegister<std::uint64_t>(WHvX64RegisterRflags);
	if (!is_debug_v) flags_v &= ~kTrapFlag;
	else             flags_v |=  kTrapFlag;
	WIN32_ERROR_ASSERT(SetRegister(WHvX64RegisterRflags, { .Reg64 = flags_v }));
}

auto Processor::AdvanceInstruction(WHV_VP_EXIT_CONTEXT const& vpcontext_v) const -> std::int32_t
{
	return SetRegister(WHvX64RegisterRip, { 
		.Reg64 = vpcontext_v.InstructionLength
		       + vpcontext_v.Rip
	});
}
