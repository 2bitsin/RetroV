#include <core/config.hpp>
#include <core/machine.hpp>

using core::Config;

void Config::SetMemorySize(std::size_t memory_size_bytes_v)
{
	if (memory_size_bytes_v < 512u*1024u) {
		memory_size_bytes_v = 512u*1024u;
	}
	m_MemorySize = (memory_size_bytes_v + 0xFFFu) & ~0xFFFu;
}

void Config::SetBootROM(std::uint64_t base_v, std::filesystem::path const& path_v)
{
	m_BootROM.second = path_v;	
	m_BootROM.first = base_v;
}

void Config::AddOptionROM(std::uint64_t base_v, std::filesystem::path const& path_v)
{
	m_OptionROMs.emplace_back(base_v, path_v);
}

auto core::Config::AddProcessor(std::uint32_t processor_v) -> void
{
	m_Processors.push_back(processor_v);
}

void Config::ApplyBeforeSetup(Machine& machine_v) const
{
	auto const processor_count_v = std::max<std::uint32_t>(1u, m_Processors.size());
	machine_v.SetProperty(WHvPartitionPropertyCodeProcessorCount, processor_count_v);
}

void Config::ApplyAfterSetup(Machine& machine_v) const
{
	// Concifure Memory
	auto base_memory_size_v = std::max<std::size_t>(640u * 1024u, m_MemorySize);
	auto index_v = machine_v.InitializeMemory(m_MemorySize);
	machine_v.MapMemory(index_v, 0, base_memory_size_v, machine_v.kMemoryFlagsRAM, 0u);
	if (m_MemorySize > base_memory_size_v) {
		auto extended_memory_size_v = m_MemorySize - base_memory_size_v;
		machine_v.MapMemory(index_v, 1024u * 1024u, extended_memory_size_v,
			machine_v.kMemoryFlagsRAM, base_memory_size_v);
	}

	// Configure BIOS
	auto [boot_base_v, boot_path_v] = m_BootROM;
	index_v = machine_v.InitializeMemory(boot_path_v);
	machine_v.MapMemory(index_v, boot_base_v, 0u, machine_v.kMemoryFlagsROM, 0u);

	// Configure Option ROMs
	for (auto const& [base_v, path_v] : m_OptionROMs) {
		index_v = machine_v.InitializeMemory(path_v);
		machine_v.MapMemory(index_v, base_v, 0u, machine_v.kMemoryFlagsROM, 0u);
	}

	// Configure Graphics Video Memory
	index_v = machine_v.InitializeMemory(64_KiB);
	machine_v.MapMemory(index_v, 0xA0000u, 0u, machine_v.kMemoryFlagsRAM, 0u);

	// Configure Text Video Memory
	index_v = machine_v.InitializeMemory(32_KiB);
	machine_v.MapMemory(index_v, 0xB8000u, 0u, machine_v.kMemoryFlagsRAM, 0u);

	// Configure Processors
	if (!m_Processors.empty()) {
		for (auto processor_v : m_Processors) {
			machine_v.InitializeProcessor(processor_v);
		}
	} else {
		machine_v.InitializeProcessor(0u);
	}
}
