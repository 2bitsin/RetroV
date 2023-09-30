#include <core/processor.hpp>
#include <core/constants.hpp>
#include <core/machine.hpp>

#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>
#include <win32/scope_name.hpp>
#include <win32/waitabletimer.hpp>

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
	if (m_Machine.GetPartition().IsMapped(physaddr_v)) {
		result_v = WHvProcessor::MemoryAccess(is_write_v, physaddr_v, data_v);
		if (ERROR_SUCCESS==result_v)
			return result_v;
	}
	result_v = m_Machine.MemoryAccess(*this, is_write_v, physaddr_v, data_v);		
	if (ERROR_SUCCESS==result_v)
		return result_v;
	__debugbreak();
	return ERROR_ACCESS_DENIED;
}

auto Processor::TranslateAddress(std::uint64_t vaddress_v, core::Access access_v) const -> std::tuple<std::int32_t, std::uint64_t>
{
	if (!PagingEnabled()) return { ERROR_SUCCESS, vaddress_v };	
	using enum core::Access;
	WHV_TRANSLATE_GVA_FLAGS flags_v{ };
	if (access_v & kAccessWrite) flags_v |= WHvTranslateGvaFlagValidateWrite;
	if (access_v & kAccessFetch) flags_v |= WHvTranslateGvaFlagValidateRead;
	auto const [status_v, code_v, paddress_v] = WHvProcessor::TranslateGva(vaddress_v, flags_v);
	if (ERROR_SUCCESS != status_v)
		return { status_v, 0u };
	if (WHvTranslateGvaResultSuccess != code_v)
		return { ERROR_ACCESS_DENIED, 0u };
	return { status_v, paddress_v };
}

auto Processor::HypercallDispatch(WHV_RUN_VP_EXIT_CONTEXT const& context_v) const -> std::int32_t
{
	HypercallContext hypercall_v{ };
	auto status_v = HypercallFunction(context_v, 
		std::ref(hypercall_v));
	if (status_v != ERROR_SUCCESS)
		return status_v;
	return m_Machine.Hypercall(*this, hypercall_v);
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

		auto regs_v = RegistersScoped<win32::regs::GeneralPurpose>();

		switch (context_v.ExitReason)
		{
		case WHvRunVpExitReasonX64IoPortAccess:
			emulator_v.TryIoEmulation(*this, context_v.VpContext, context_v.IoPortAccess);
			continue;
		case WHvRunVpExitReasonMemoryAccess:		
			emulator_v.TryMmioEmulation(*this, context_v.VpContext, context_v.MemoryAccess);
			continue;		
		case WHvRunVpExitReasonHypercall:
			HypercallDispatch(context_v);
			status_v = AdvanceInstruction(context_v.VpContext);
			if (ERROR_SUCCESS != status_v)
				return { status_v, context_v };
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
			if (auto const value_o = 
				m_IjQueue.try_pop(); 
				value_o.has_value()) 
			{
				(*value_o)(*this);		
				continue;
			}
			m_Suspend.acquire();
			continue;

		case WHvRunVpExitReasonX64MsrAccess:
			s_log.UnhandledMSR(GetIndex(), context_v.VpContext, context_v.MsrAccess);
			status_v = AdvanceInstruction(context_v.VpContext);
			if (ERROR_SUCCESS != status_v)
				return { status_v, context_v };
			continue;
		case WHvRunVpExitReasonException:
			s_log.UnhandledException(GetIndex(), context_v.VpException, context_v.VpContext);
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

auto Processor::Interject(interjection_type what_v) -> void
{
	m_IjQueue.emplace(what_v);
	WHvProcessor::Cancel();
}

auto Processor::Resume()  -> void
{
	m_Suspend.release();
}

auto Processor::GetRuntime() const -> std::tuple<std::int32_t, std::uint64_t>
{
	WHV_REGISTER_VALUE value_v{};
	auto result_v = GetRegister(WHvRegisterVpRuntime, value_v);
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

auto Processor::PagingEnabled() const -> bool
{
	auto const cr0r_v = GetRegister<uint64_t>(WHvX64RegisterCr0);
	auto const mask_v = 0x80000001u;
	return mask_v == (cr0r_v & mask_v);
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

auto Processor::HypercallFunction(WHV_RUN_VP_EXIT_CONTEXT const& context_v, HypercallContext& output_v) const -> std::int32_t
{
	auto const& vpcontext_v = context_v.VpContext;
	auto const& hypercall_v = context_v.Hypercall;

	if (vpcontext_v.ExecutionState.Cpl != 0)
		return ERROR_ACCESS_DENIED;

	output_v.VpContext = vpcontext_v;
	output_v.Hypercall = hypercall_v;
	output_v.Function = (std::uint16_t)hypercall_v.Rax;
	output_v.RaxUsed = 1;

	if (auto iaddress_v = vpcontext_v.Cs.Base + vpcontext_v.Rip; 
		iaddress_v >= 3u)
	{
		iaddress_v -= 3u;	
		if (PagingEnabled()) 
		{
			auto const [status_v, result_v, oaddress_v] = 
				TranslateGva(iaddress_v, WHvTranslateGvaFlagNone);
			if (status_v != ERROR_SUCCESS)
				return status_v;
			iaddress_v = oaddress_v;
		}
		auto const opcode_v = MemoryFetch<uint8_t>(iaddress_v); 
		iaddress_v += 1u;
		if (0x68==opcode_v) {
			output_v.Function = MemoryFetch<uint16_t>(iaddress_v);
			output_v.RaxUsed = 0;
		}		
	}
	return ERROR_SUCCESS;
}