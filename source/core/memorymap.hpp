#pragma once

#include <cstddef>
#include <cstdint>

#include <mutex>
#include <shared_mutex>

#include <utils/objects.hpp>
#include <utils/interval_map.hpp>
#include <core/memory.hpp>
#include <core/accessflags.hpp>

namespace core
{
	struct VirtualMachineBase;

	struct MemoryMap
		: public utils::uncopyable
		, public utils::unmovable
	{
		static inline constexpr const auto kPageSize = Memory::kPageSize;
		static inline constexpr const auto kLastAddress = 0xFFFFFFFFFFFFFFFFull;
		static inline constexpr const auto kPageLimit = (kLastAddress >> 12u)+1u;

		MemoryMap(VirtualMachineBase& vm_base_v);

		auto DefineRegion(uint64_t base_page_v, Access access_v, Memory& block_v, uint64_t page_count_v = 0u) -> std::size_t;
		auto DefineRegion(uint64_t base_page_v, Access access_v, std::unique_ptr<Memory> block_v, uint64_t page_count_v = 0u) -> std::size_t;
		auto RemoveRegion(std::size_t region_index_v) -> void;

	private:

		using AccessFun = void(std::uint64_t, bool, std::uint8_t, std::uint8_t (&) [8u]);
		std::shared_mutex m_Mutex;

	#pragma pack(push, 1)
		enum RegionFlags : std::uint64_t {
			kReleaseAfterDone = 0x1u
		};

		struct region_type {			
			Memory* m_Memory;
			AccessFun* m_AccessFun;
			Access m_Access:12;
			uint64_t m_PageCount:52;
			uint64_t m_BasePage:52;
			RegionFlags m_Flags:12;
		};
		static_assert(sizeof(region_type) == 32u, "I want to fit in a cache line");
	#pragma pack(pop)
	
		utils::interval_map<uint64_t, std::size_t> m_Map;
		std::vector<region_type> m_Regions;
		std::vector<std::size_t> m_Free;
	};
}