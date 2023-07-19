#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>

#include <mutex>
#include <shared_mutex>

#include <utils/objects.hpp>
#include <utils/interval_map.hpp>
#include <utils/bitmanip.hpp>
#include <core/memory.hpp>
#include <core/accessflags.hpp>
#include <core/constants.hpp>

namespace core
{
	struct VirtualMachine;

	struct MemoryMap
	{		
		auto operator=(MemoryMap const&) -> MemoryMap& = delete;
		auto operator=(MemoryMap &&) -> MemoryMap& = delete;
		MemoryMap(MemoryMap const&) = delete;
		MemoryMap(MemoryMap&&) = delete;

		MemoryMap(VirtualMachine& vm_base_v);

		auto DefineRegion(uint64_t base_page_v, Access access_v, Memory& block_v, uint64_t pages_v = 0u) -> std::size_t;
		auto DefineRegion(uint64_t base_page_v, Access access_v, std::unique_ptr<Memory> block_v, uint64_t pages_v = 0u) -> std::size_t;
		auto RemoveRegion(std::size_t index_v) -> void;	
		auto MemoryAccess(bool is_write_v, uint64_t address_v, std::uint8_t size_v, utils::bytes<8u>& data_v) const -> std::uint32_t;		
		auto MemoryView(std::uint64_t address_v, std::uint64_t size_v) const -> std::span<std::byte>;
	protected:

		enum RegionFlags: uint32_t {
			kNone = 0x0u,
			kReleaseAfterDone = 0x1u,
			kMappedDirectly = 0x2u
		};

		HVDOS_DEFINE_FRIEND_ENUM_FLAG_OPERATORS(RegionFlags)

	#pragma pack(push, 1)
		struct region_type {			
			Memory* m_Memory;
			std::byte* m_Data;
			uint64_t m_PageCount:52;
			uint64_t m_Access:12;
			uint64_t m_BasePage:52;
			uint64_t m_Flags:12;
		};

	#pragma pack(pop)
		static_assert((64u % sizeof(region_type) == 0u)
			          ||(sizeof(region_type) % 64u == 0u)
			           ,"cache line aligned");

		auto AllocateRegion() -> std::size_t ;
		auto DefineRegion(uint64_t base_page_v, Access access_v, std::uint16_t flags_v, Memory* block_v, uint64_t page_count_v) -> std::size_t;


	private:
		VirtualMachine& m_VMBase;
		mutable std::shared_mutex m_Mutex;
		utils::interval_map<uint64_t, std::size_t> m_AddressMap;
		std::vector<region_type> m_Regions;
		std::vector<std::size_t> m_FreeRegions;
	};
}