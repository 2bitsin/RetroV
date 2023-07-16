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

auto MemoryMap::DefineRegion(uint64_t base_page_v, Access access_v, RegionFlags flags_v, Memory* block_v, uint64_t pages_v) -> std::size_t
{
	pages_v=!pages_v?(block_v->Size()/kPageSize):pages_v;
	auto const index_v = AllocateRegion();
	std::unique_lock lock_v{ m_Mutex };
	auto& region_v = m_Regions[index_v];
	region_v.m_Memory = block_v;
	region_v.m_PageCount = pages_v;
	region_v.m_Access = access_v;
	region_v.m_BasePage = base_page_v;	
	auto base_v = base_page_v*kPageSize;
	auto last_v = base_v + pages_v*kPageSize;
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

}
