#include <core/processor.hpp>
#include <core/machine.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>

using core::Processor;

Processor::Processor(Machine& machine_v, std::uint32_t vcpuindex_v)
	: WHvProcessor{ machine_v.Partition(), vcpuindex_v }
	, m_Machine{ machine_v }
	, m_Halt{ 0u }
{}

Processor::~Processor()
{}

auto Processor::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t
{
	return m_Machine.IoPortAccess(is_write_v, port_v, data_v);
}

auto Processor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t
{	
	return WHvProcessor::MemoryAccess(is_write_v, physaddr_v, data_v);
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

auto Processor::Run(std::stop_token stopper_v) const -> std::tuple<std::int32_t, WHV_RUN_VP_EXIT_CONTEXT>
{
	std::stop_callback stopcbk_v(stopper_v, [this] { 
		m_Halt.release();
		Cancel();
	});
	auto& emulator_v = Emulator();
	std::int32_t status_v{ 0 };
	WHV_RUN_VP_EXIT_CONTEXT context_v{};
	while (!stopper_v.stop_requested())
	{		
		WIN32_ERROR_ASSERT(std::get<std::int32_t&>(
			std::tie(status_v, context_v) = WHvProcessor::Run()
		));
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
				m_Halt.acquire();
				continue;
			}			
			[[fallthrough]];
		case WHvRunVpExitReasonCanceled:
			return { status_v, context_v };
		default:
			__debugbreak();
			return { status_v, context_v };
		}
	}
	return { status_v, context_v };
}

auto Processor::RequestInterrupt(std::uint8_t vector_v) const -> std::int32_t
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
	m_Halt.release();
	return result_v;
}

auto Processor::RequestNonMaskable() const -> std::int32_t
{
	return RequestInterrupt(2u);
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
