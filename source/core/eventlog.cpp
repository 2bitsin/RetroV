#include <cassert>

#include <core/eventlog.hpp>
#include <utils/logger.hpp>
#include <utils/algorithm.hpp>
#include <win32/winhvpx.hpp>

using core::EventLog;

using utils::logger;

using p = std::uintptr_t;

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
	logger::trace(logger::deflog, "Starting machine...");
}

auto EventLog::StopMachine() const -> void
{
	using utils::logger;
	logger::trace(logger::deflog, "Stopping machine...");
}

auto EventLog::ResetMachine() const -> void
{
	using utils::logger;
	logger::trace(logger::deflog, "Resetting machine...");
}

auto EventLog::MapGpaRange(void* addr_v, std::uint64_t base_v, std::uint64_t size_v, Access access_v) const -> void
{	
	assert(addr_v != nullptr);
	logger::trace(logger::deflog, "Mapping {:#018x} ... {:#018x} -> {:#018x} | {}", base_v, base_v + size_v, (p)addr_v, to_string(access_v));
}

auto EventLog::MapGpaRangeFromFile(void* addr_v, std::uint64_t base_v, std::uint64_t size_v, std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v) const -> void
{
	assert(addr_v != nullptr);
	std::string string_path_v = std::filesystem::relative(path_v).string();
	if (string_path_v.empty() && !path_v.empty())
		string_path_v = path_v.string();
	logger::trace(logger::deflog, "Mapping {:#018x} ... {:#018x} -> {}[{:#x}:{:#x}]",
		base_v, base_v+size_v, string_path_v, offset_v, length_v);
}

auto EventLog::UnmapGpaRange(std::uint64_t base_v, std::uint64_t size_v) const -> void
{
	logger::trace(logger::deflog, "Unmapping {:#016x} ... {:#016x}", base_v, base_v + size_v);
}

auto EventLog::UnrealModeEnabled(std::uint32_t vcpu_index_v) const -> void
{
	logger::info(logger::deflog, "CPU[{}] flat real mode hack enabled!", vcpu_index_v);
}

auto EventLog::UnrealModeDisabled(std::uint32_t vcpu_index_v) const -> void
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

auto EventLog::VCpuExited(std::uint32_t vcpu_index_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_context_v) const -> void
{
	auto const reason_v = to_string(exit_context_v.ExitReason);
	logger::debug(logger::deflog, "CPU[{}] exited with reason '{}'", vcpu_index_v, reason_v);
}

auto EventLog::IRQState(std::uint32_t vcpu_index_v, std::uint16_t state_v) const -> void
{
	using utils::logger;
	logger::info(logger::deflog, "CPU[{}] raised IRQ [{}]", vcpu_index_v, utils::bitset_to_string(state_v));
}

EventLog::EventLog(std::string_view name_v)
	: m_Module (std::string(name_v))
{}

