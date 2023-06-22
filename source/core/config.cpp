#include <core/config.hpp>
#include <core/machine.hpp>

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

void Config::ApplyBeforeSetup(Machine& machine_v) const
{
	// Concifure Memory
	auto base_memory_size_v = std::max<std::size_t>(640u*1024u, m_MemorySize);
	auto index_v = machine_v.InitializeMemory(m_MemorySize);
	machine_v.MapMemory(index_v, 0, base_memory_size_v, machine_v.kMemoryFlagsRAM, 0u);
	if (m_MemorySize > base_memory_size_v) {
		auto extended_memory_size_v = m_MemorySize - base_memory_size_v;
		machine_v.MapMemory(index_v, 1024u*1024u, extended_memory_size_v, 
			machine_v.kMemoryFlagsRAM, base_memory_size_v);
	}
}

void Config::ApplyAfterSetup(Machine& machine_v) const
{
}
