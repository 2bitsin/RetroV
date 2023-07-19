#include <core/memorymap.hpp>
#include <core/virtualmachine.hpp>
#include <core/constants.hpp>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

using core::MemoryMap;

MemoryMap::MemoryMap(VirtualMachine& vm_base_v)
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
		pages_v = std::min<std::size_t>(pages_v, block_v->Size() / kPageSize);
	}

	std::unique_lock lock_v{ m_Mutex };

	auto const index_v = AllocateRegion();

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
		m_VMBase.Partition().MapGpaRange(region_v.m_Data, base_v, size_v, access_v);
	}
	m_AddressMap.insert({base_v, last_v}, index_v);
	return index_v;
}


auto MemoryMap::AllocateRegion() -> std::size_t
{	
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
		m_VMBase.Partition().UnmapGpaRange(base_v, size_v);
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

auto MemoryMap::MemoryAccess(bool is_write_v, uint64_t address_v, std::uint8_t size_v, utils::bytes<8u>& data_v) const -> std::uint32_t
{
	std::shared_lock lock_v{ m_Mutex };

	auto const index_v = m_AddressMap.value_at(address_v);
	auto const& region_v = m_Regions[index_v];
	auto const offset_v = address_v - region_v.m_BasePage*kPageSize;

	if (is_write_v) {
		if (!(region_v.m_Access & kAccessWrite)) {
			return E_ACCESSDENIED;
		}
	} else{
		if (!(region_v.m_Access & kAccessExecute)
			&&!(region_v.m_Access & kAccessFetch)) {
			return E_ACCESSDENIED;
		}
	}
	if (offset_v >= region_v.m_PageCount*kPageSize) {
		return E_ACCESSDENIED;
	}

	auto const block_size_v = region_v.m_PageCount * kPageSize;

	size_v = (std::uint8_t)std::min<std::size_t>(size_v, block_size_v - offset_v);

	if (!region_v.m_Data) {
		auto& mem_v = *region_v.m_Memory;
		return mem_v.Access(offset_v, is_write_v, size_v, data_v);
	}

	if (is_write_v) {
		std::memcpy(region_v.m_Data + offset_v, std::data(data_v), size_v);
	} else {
		std::memcpy(std::data(data_v), region_v.m_Data + offset_v, size_v);
	}	
	return S_OK;
}

auto MemoryMap::MemoryView(std::uint64_t address_v, std::uint64_t size_v) const 
	-> std::span<std::byte> 
{
	std::shared_lock lock_v{ m_Mutex };

	auto const index_v = m_AddressMap.value_at(address_v);
	auto const& region_v = m_Regions[index_v];
	auto const offset_v = address_v - region_v.m_BasePage*kPageSize;

	if (offset_v >= region_v.m_PageCount*kPageSize) {
		return {};
	}

	auto const block_size_v = region_v.m_PageCount*kPageSize;
	size_v = std::min<std::size_t>(size_v, block_size_v - offset_v);

	if (!region_v.m_Data) {
		auto& mem_v = *region_v.m_Memory;
		return mem_v.Data(offset_v, size_v);
	}

	return{ region_v.m_Data + offset_v, size_v };
}
 