#include <core/config.hpp>
#include <core/hypervisor.hpp>

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

auto core::Config::AddDevice(std::string_view name_v, std::any config_v)
{
}

void Config::ApplyBeforeSetup(Hypervisor& hypervisor_v) const
{
	auto const processor_count_v = std::max<std::uint32_t>(1u, m_Processors.size());
	auto& partition_v = hypervisor_v.GetPartition();
	partition_v.SetProperty(WHvPartitionPropertyCodeProcessorCount, processor_count_v);
}

void Config::ApplyAfterSetup(Hypervisor& hypervisor_v) const
{
	// Concifure Memory
	auto base_memory_size_v = std::min<std::size_t>(640u * 1024u, m_MemorySize);
	auto& pool_v = hypervisor_v.GetMemPool();
	auto index_v = pool_v.AllocateBlock(m_MemorySize);
	auto& memory_v = hypervisor_v.GetMemManager();
	memory_v.MapPhysical(index_v, 0, base_memory_size_v, memory_v.kMemoryFlagsRAM, 0u);
	auto extended_memory_size_v = m_MemorySize - base_memory_size_v;
	if (m_MemorySize > base_memory_size_v) {		
		memory_v.MapPhysical(index_v, 1024u * 1024u, extended_memory_size_v,
			memory_v.kMemoryFlagsRAM, base_memory_size_v);
	}

	// Configure BIOS
	auto [boot_base_v, boot_path_v] = m_BootROM;
	memory_v.MapPhysical(pool_v.AllocateBlock(boot_path_v), boot_base_v, 0u, memory_v.kMemoryFlagsROM, 0u);

	// Configure Option ROMs
	for (auto const& [base_v, path_v] : m_OptionROMs) {
		memory_v.MapPhysical(pool_v.AllocateBlock(path_v), base_v, 0u, memory_v.kMemoryFlagsROM, 0u);
	}

	// Configure Processors
	if (!m_Processors.empty()) {
		for (auto&& processor_v : m_Processors) {
			hypervisor_v.InitializeProcessor(processor_v);
		}
	} else {
		hypervisor_v.InitializeProcessor(0u);
	}
}
