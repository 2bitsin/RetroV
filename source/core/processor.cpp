#include <core/processor.hpp>
#include <core/machine.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>

#include <utils/literals.hpp>
#include <utils/lambda.hpp>

#include <future>

using namespace size_literals;

using core::Processor;

Processor::Processor(Machine& machine_v, std::uint32_t vcpuindex_v)
	: WHvProcessor{ machine_v.Partition(), vcpuindex_v }
	, m_Machine{ machine_v }
	, m_Suspend{ 0u }
{}

Processor::~Processor()
{}

auto Processor::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t
{
	return m_Machine.IoPortAccess(is_write_v, port_v, data_v);
}

auto Processor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t
{	
	using T = WHvProcessor;
	if (auto result_v = T::MemoryAccess(is_write_v, physaddr_v, data_v); result_v) {
		if (ERROR_ACCESS_DENIED == HRESULT_CODE(result_v))
			return m_Machine.MemoryAccess(is_write_v, physaddr_v, data_v);		
		__debugbreak();
		return result_v;
	}			
	return ERROR_SUCCESS;
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

auto Processor::Run(std::stop_token stoppee_v) -> exit_result_type
{
	std::stop_callback stopcbk_v{ stoppee_v, [this] { 
		WHvProcessor::Cancel();
		m_Suspend.release();
	}};

	std::unique_lock lock_v{ m_IsRunning };
	auto& emulator_v = Emulator();
	while (!stoppee_v.stop_requested())
	{		
		auto const result_v = WHvProcessor::Run();
		auto const [status_v, context_v] = result_v;
		if (!SUCCEEDED(status_v))
			return result_v;		
		switch (context_v.ExitReason)
		{
		case WHvRunVpExitReasonX64IoPortAccess:
			emulator_v.TryIoEmulation(*this, context_v.VpContext, context_v.IoPortAccess);
			continue;
		case WHvRunVpExitReasonX64MsrAccess:
			emulator_v.TryMmioEmulation(*this, context_v.VpContext, context_v.MemoryAccess);
			continue;		
		case WHvRunVpExitReasonX64Halt:
			if (InterruptsEnabled()) { 
				m_Suspend.acquire();
				continue;
			}			
			[[fallthrough]];
		case WHvRunVpExitReasonCanceled:
			return result_v;
		default:
			return result_v;
		}
	}
	// Discard injected cacel event
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

auto Processor::RequestInterrupt(std::uint8_t vector_v) -> std::int32_t
{	
	auto const result_v = SetRegister(WHvRegisterPendingInterruption, {
		.PendingInterruption = {
			.InterruptionPending = 1u,
			.InterruptionType = WHvX64PendingInterrupt,
			.DeliverErrorCode = 0u,
			.InstructionLength = 0u,
			.NestedEvent = 0u,
			.InterruptionVector = vector_v,
			.ErrorCode = 0u
		}
	});
	m_Suspend.release();
	return result_v;
}

auto Processor::RequestNonMaskable() -> std::int32_t
{
	auto const result_v = SetRegister(WHvRegisterPendingInterruption, {
		.PendingInterruption = {
			.InterruptionPending = 1u,
			.InterruptionType = WHvX64PendingNmi,
			.DeliverErrorCode = 0u,
			.InstructionLength = 0u,
			.NestedEvent = 0u,
			.InterruptionVector = 2u,
			.ErrorCode = 0u
		}
		});
	m_Suspend.release();
	return result_v;
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
