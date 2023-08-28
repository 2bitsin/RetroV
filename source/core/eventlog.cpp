#include <core/eventlog.hpp>

#include <utils/logger.hpp>


using core::EventLog;
using utils::logger;
using p = std::uintptr_t;

auto EventLog::MapGpaRange(void* addr_v, std::uint64_t base_v, std::uint64_t size_v, Access access_v) const -> void
{	
	logger::trace(logger::deflog, "Mapping {:#016x} ... {:#016x} -> {:#016x} | {}", base_v, base_v + size_v, (p)addr_v, to_string(access_v));
}

auto EventLog::MapGpaRangeFromFile(void* addr_v, std::uint64_t base_v, std::uint64_t size_v, std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v) const -> void
{
	
	std::string string_path_v = std::filesystem::relative(path_v).string();
	logger::trace(logger::deflog, "Mapping {:#016x} ... {:#016x} -> {}[{:#x}:{:#x}]",
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

auto EventLog::EmitPostCode(utils::limited_span<std::byte, 4u> data_v) const -> void
{
	switch (data_v.size())
	{
	case 1: logger::debug(logger::deflog, "POST_CODE: {:#04x}", data_v.as<uint8_t >()); break;
	case 2: logger::debug(logger::deflog, "POST_CODE: {:#06x}", data_v.as<uint16_t>()); break;
	case 4: logger::debug(logger::deflog, "POST_CODE: {:#010x}", data_v.as<uint32_t>()); break;
	}

}

EventLog::EventLog(std::string_view name_v)
	: m_Module (std::string(name_v))
{}

