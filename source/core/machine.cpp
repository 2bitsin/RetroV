#include <win32/error.hpp>

#include <stdexcept>

#include "machine.hpp"


Machine::Machine(Config const& config_v)
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Partition));
	config_v.ApplyBeforeSetup(*this);
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_Partition));
	config_v.ApplyAfterSetup(*this);
}

Machine::~Machine()
{
	if (m_Partition) {
		WHvDeletePartition(m_Partition);
		m_Partition = nullptr;
	}
	m_Memories.clear();
}

auto Machine::MapMemory(std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t flags_v, std::uint64_t offset_v) -> void {
	if (index_v >= m_Memories.size()) {
		throw std::invalid_argument("Invalid memory index");
	}
	auto address_v = m_Memories[index_v].Data() + offset_v;
	WIN32_ERROR_ASSERT(::WHvMapGpaRange(m_Partition, address_v, base_v, size_v, (WHV_MAP_GPA_RANGE_FLAGS)flags_v));
}

void Machine::UnmapMemory(std::uint64_t base_v, std::uint64_t size_v)
{
	WIN32_ERROR_ASSERT(::WHvUnmapGpaRange(m_Partition, base_v, size_v));
}
