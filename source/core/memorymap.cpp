#include <core/memorymap.hpp>

using core::MemoryMap;

MemoryMap::MemoryMap(VirtualMachineBase& vm_base_v)
	:	m_VMBase      { vm_base_v }
	, m_AddressMap  {           }
	, m_Regions     {{ nullptr }}
	, m_FreeRegions {           }	
{}

auto MemoryMap::DefineRegion(uint64_t base_page_v, Access access_v, Memory& block_v, uint64_t pages_v) -> std::size_t
{	
	return DefineRegion(base_page_v, access_v, RegionFlags::kNone, &block_v, pages_v);
}

auto MemoryMap::DefineRegion(uint64_t base_page_v, Access access_v, std::unique_ptr<Memory> block_v, uint64_t pages_v) -> std::size_t
{
	return DefineRegion(base_page_v, access_v, RegionFlags::kReleaseAfterDone, block_v.release(), pages_v);
}

auto MemoryMap::DefineRegion(uint64_t base_page_v, Access access_v, std::uint16_t flags_v, Memory* block_v, uint64_t pages_v) -> std::size_t
{
	if (pages_v == 0u) {
		pages_v = block_v->Size() / kPageSize;
	} else {
		pages_v = std::min(pages_v, block_v->Size() / kPageSize);
	}

	auto const index_v = AllocateRegion();

	std::unique_lock lock_v{ m_Mutex };

	auto& region_v = m_Regions[index_v];

	region_v.m_Memory = block_v;
	region_v.m_PageCount = pages_v;
	region_v.m_Access = access_v;
	region_v.m_Flags = flags_v;
	region_v.m_BasePage = base_page_v;	
	region_v.m_Data = block_v->Data().data();	

	auto const base_v = base_page_v * kPageSize;
	auto const size_v = pages_v * kPageSize;
	auto const last_v = base_v + size_v;
	if (nullptr != region_v.m_Data) {
		region_v.m_Flags |= RegionFlags::kMappedDirectly;		
		//m_VMBase.Partition().MapGpaRegion(region_v.m_Data, base_v, size_v, access_v);
	}
	m_AddressMap.insert({base_v, last_v}, index_v);
	return index_v;
}


auto MemoryMap::AllocateRegion() -> std::size_t
{
	std::unique_lock lock_v{ m_Mutex };
	if (m_FreeRegions.empty()) {
		auto const index_v = m_Regions.size();
		m_Regions.emplace_back();
		return index_v; 
	} else {
		auto const index_v = m_FreeRegions.back();
		m_FreeRegions.pop_back();
		return index_v; 
	}
}

auto MemoryMap::RemoveRegion(std::size_t index_v) -> void
{
	std::unique_lock lock_v{ m_Mutex };	

	auto& region_v = m_Regions[index_v];

	auto const base_v = region_v.m_BasePage*kPageSize;
	auto const size_v = region_v.m_PageCount*kPageSize;
	auto const last_v = base_v + size_v;

	m_AddressMap.insert({base_v, last_v}, 0u);

	if (region_v.m_Flags & RegionFlags::kMappedDirectly) {
		//m_VMBase.Partition().UnmapGpaRange(base_v, size_v);
	}
	if (region_v.m_Flags & RegionFlags::kReleaseAfterDone) {
		delete region_v.m_Memory;
	}	

	region_v.m_Memory = nullptr;
	region_v.m_PageCount = 0u;
	region_v.m_Access = Access::kAccessNone;
	region_v.m_BasePage = 0u;
	region_v.m_Flags = RegionFlags::kNone;
	region_v.m_Data = nullptr;

	m_FreeRegions.push_back(index_v);

}
