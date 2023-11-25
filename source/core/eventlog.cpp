#include <cassert>

#include <win32/winhvpx.hpp>

#include <utils/algorithm.hpp>
#include <utils/logger.hpp>

#include <core/eventlog.hpp>

using core::EventLog;

using utils::logger;

using p = uintptr_t;

static inline auto to_string(WHV_RUN_VP_EXIT_REASON reason_v) -> std::string_view {
	switch (reason_v) 
	{
#define X(x) case x: return #x
	X(WHvRunVpExitReasonNone);
	X(WHvRunVpExitReasonMemoryAccess);
	X(WHvRunVpExitReasonX64IoPortAccess);
	X(WHvRunVpExitReasonUnrecoverableException);
	X(WHvRunVpExitReasonInvalidVpRegisterValue);
	X(WHvRunVpExitReasonUnsupportedFeature);
	X(WHvRunVpExitReasonX64InterruptWindow);
	X(WHvRunVpExitReasonX64Halt);
	X(WHvRunVpExitReasonX64ApicEoi);
	X(WHvRunVpExitReasonSynicSintDeliverable);
	X(WHvRunVpExitReasonX64MsrAccess);
	X(WHvRunVpExitReasonX64Cpuid);
	X(WHvRunVpExitReasonException);
	X(WHvRunVpExitReasonX64Rdtsc);
	X(WHvRunVpExitReasonX64ApicSmiTrap);
	X(WHvRunVpExitReasonHypercall);
	X(WHvRunVpExitReasonX64ApicInitSipiTrap);
	X(WHvRunVpExitReasonX64ApicWriteTrap);
	X(WHvRunVpExitReasonCanceled);
#undef X
	}
	return "(Unknown)";
}

auto EventLog::StartMachine() const -> void
{
	using utils::logger;
	logger::debug(logger::deflog, "Starting machine...");
}

auto EventLog::StopMachine() const -> void
{
	using utils::logger;
	logger::debug(logger::deflog, "Stopping machine...");
}

auto EventLog::ResetMachine() const -> void
{
	using utils::logger;
	logger::debug(logger::deflog, "Resetting machine...");
}

auto EventLog::MapGpaRange(void* addr_v, uint64_t base_v, uint64_t size_v, Access access_v) const -> void
{	
	assert(addr_v != nullptr);
	logger::debug(logger::deflog, "Mapping {:#018x} ... {:#018x} -> {:#018x} | {}", base_v, base_v + size_v, (p)addr_v, to_string(access_v));
}

auto EventLog::MapGpaRangeFromFile(void* addr_v, uint64_t base_v, uint64_t size_v, std::filesystem::path const& path_v, uint64_t offset_v, uint64_t length_v) const -> void
{
	assert(addr_v != nullptr);
	std::string string_path_v = std::filesystem::relative(path_v).string();
	if (string_path_v.empty() && !path_v.empty())
		string_path_v = path_v.string();
	logger::debug(logger::deflog, "Mapping {:#018x} ... {:#018x} -> {}[{:#x}:{:#x}]",
		base_v, base_v+size_v, string_path_v, offset_v, length_v);
}

auto EventLog::UnmapGpaRange(uint64_t base_v, uint64_t size_v) const -> void
{
	logger::debug(logger::deflog, "Unmapping {:#016x} ... {:#016x}", base_v, base_v + size_v);
}

auto EventLog::UnrealModeEnabled(uint32_t vcpu_index_v) const -> void
{
	logger::info(logger::deflog, "CPU[{}] flat real mode hack enabled!", vcpu_index_v);
}

auto EventLog::UnrealModeDisabled(uint32_t vcpu_index_v) const -> void
{
	logger::info(logger::deflog, "CPU[{}] flat real mode hack disabled!", vcpu_index_v);
}

auto EventLog::EmitPostCode(utils::limited_span<std::byte, 4u> data_v) const -> void
{
	switch (data_v.size())
	{
	case 1: logger::debug(logger::deflog, "POST_CODE: {:#04x}", data_v.as<uint8_t >()); break;
	case 2: logger::debug(logger::deflog, "POST_CODE: {:#06x}", data_v.as<uint16_t>()); break;
	case 4: logger::debug(logger::deflog, "POST_CODE: {:#010x}", data_v.as<uint32_t>()); break;
	}
}

auto EventLog::VCpuExited(uint32_t vcpu_index_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_context_v) const -> void
{
	auto const reason_v = to_string(exit_context_v.ExitReason);
	logger::debug(logger::deflog, "CPU[{}] exited with reason '{}'", vcpu_index_v, reason_v);
}

auto EventLog::IRQState(uint32_t vcpu_index_v, uint16_t state_v) const -> void
{
	logger::info(logger::deflog, "CPU[{}] raised IRQ [{}]", vcpu_index_v, utils::bitset_to_string(state_v));
}

auto EventLog::DebugTrap(uint32_t vcpu_index_v, uint64_t linaddr_v, uint16_t segsel_v, uint64_t offset_v) const -> void
{
	logger::debug(logger::deflog, "CPU[{}] : DebuggerBreak at ({:08x}) with CS={:04x} IP={:08x}", vcpu_index_v, linaddr_v, segsel_v, offset_v);
}

auto EventLog::UnhandledException(uint32_t vcpuindex_v, const WHV_VP_EXCEPTION_CONTEXT& exception_v, const WHV_VP_EXIT_CONTEXT& context_v) const -> void
{
	logger::error(logger::deflog, "CPU[{}] raised exception: {:d}({:#04X}) at {:04X}:{:08X}.", vcpuindex_v, exception_v.ExceptionType, exception_v.ExceptionType, context_v.Cs.Selector, context_v.Rip);
}

auto EventLog::UnhandledMSR(uint32_t vcpu_index_v, WHV_VP_EXIT_CONTEXT const& context_v, WHV_X64_MSR_ACCESS_CONTEXT const& access_v) const -> void
{
	using utils::logger;
	if (access_v.AccessInfo.IsWrite) {
		logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) write at {:#06x}:{:#010x}, EDX:EAX={:010X}:{:010X}", vcpu_index_v, access_v.MsrNumber, context_v.Cs.Selector, context_v.Rip, access_v.Rdx, access_v.Rax);
	}
	else {
		logger::error(logger::deflog, "CPU[{}] Unhandled MSR({:#010x}) read at {:#06x}:{:#010x}", vcpu_index_v, access_v.MsrNumber, context_v.Cs.Selector, context_v.Rip);
	}
}

auto EventLog::EmulatorFailed(WHV_RUN_VP_EXIT_CONTEXT const& context_v, int32_t status_v, WHV_EMULATOR_STATUS emulator_status_v) const -> void
{
	using namespace std::literals;
	std::vector<std::string_view> failed_v;

	if (emulator_status_v.EmulationSuccessful) 
		failed_v.emplace_back("EmulationSuccessful"sv);
	if (emulator_status_v.InternalEmulationFailure) 
		failed_v.emplace_back("InternalEmulationFailure"sv);
	if (emulator_status_v.IoPortCallbackFailed) 
		failed_v.emplace_back("IoPortCallbackFailed"sv);
	if (emulator_status_v.MemoryCallbackFailed) 
		failed_v.emplace_back("MemoryCallbackFailed"sv);
	if (emulator_status_v.TranslateGvaPageCallbackFailed) 
		failed_v.emplace_back("TranslateGvaPageCallbackFailed"sv);
	if (emulator_status_v.TranslateGvaPageCallbackGpaIsNotAligned) 
		failed_v.emplace_back("TranslateGvaPageCallbackGpaIsNotAligned"sv);
	if (emulator_status_v.GetVirtualProcessorRegistersCallbackFailed) 
		failed_v.emplace_back("GetVirtualProcessorRegistersCallbackFailed"sv);
	if (emulator_status_v.SetVirtualProcessorRegistersCallbackFailed) 
		failed_v.emplace_back("SetVirtualProcessorRegistersCallbackFailed"sv);
	if (emulator_status_v.InterruptCausedIntercept) 
		failed_v.emplace_back("InterruptCausedIntercept"sv);
	if (emulator_status_v.GuestCannotBeFaulted) 
		failed_v.emplace_back("GuestCannotBeFaulted"sv);


	
	std::span<uint8_t const> data_v { };
	switch(context_v.ExitReason)
	{
	case WHvRunVpExitReasonMemoryAccess:
		data_v = std::span{ context_v.MemoryAccess.InstructionBytes, 
			context_v.MemoryAccess.InstructionByteCount };		
		break;
	case WHvRunVpExitReasonX64IoPortAccess:
		data_v = std::span{ context_v.IoPortAccess.InstructionBytes,
			context_v.IoPortAccess.InstructionByteCount };
		break;
	default:		
		break;
	}

	std::string bytes_as_hex_v;
	for (auto const byte_v : data_v) {
		if (bytes_as_hex_v.size() > 0u)
			bytes_as_hex_v += ","sv;
		bytes_as_hex_v += std::format("{:#04x}", byte_v);
	}
	
	logger::error(logger::deflog, "{:s} failed with STATUS={:#010x}, WHV_EMULATOR_STATUS={{{:s}}} InstructionBytes={{{:s}}}"sv, 
		to_string(context_v.ExitReason), status_v, utils::join(failed_v, "|"sv), bytes_as_hex_v);
}

EventLog::EventLog(std::string_view name_v)
	: m_Module (std::string(name_v))
{}

