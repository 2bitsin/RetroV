#include <win32/Hresult.hpp>
#include <hvisor/hypervisor.hpp>

static inline auto WHvSetPartitionProperty_(auto&& partition_v, auto&& code_v, auto const& data_v) {
	return WHvSetPartitionProperty(partition_v, code_v, &data_v, sizeof(data_v));
}

static inline auto assert_(auto result_v) -> void {
	if (result_v != S_OK) throw win32_error(result_v);
}

HyperVisor::HyperVisor() 	
	:	m_partition { nullptr }
{
	static constexpr auto every_permission_s = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute;

	assert_(WHvCreatePartition(&m_partition));
	assert_(WHvSetPartitionProperty_(m_partition, WHvPartitionPropertyCodeProcessorCount, std::uint32_t{ 1u }));
	assert_(WHvSetPartitionProperty_(m_partition, WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .AsUINT64 = 0 }));
	assert_(WHvSetupPartition(m_partition));
	assert_(WHvCreateVirtualProcessor(m_partition, 0, 0));
}

HyperVisor::~HyperVisor() 
{
	if (!m_partition) return;
	WHvDeletePartition(m_partition);
}

auto HyperVisor::MapPhysical(std::span<std::byte> source_v, std::uint64_t destination_v, WHV_MAP_GPA_RANGE_FLAGS flags_v) -> void {
	assert_(WHvMapGpaRange(m_partition, source_v.data(), destination_v, source_v.size(), flags_v));
}

