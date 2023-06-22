#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include <win32/winhvpx.hpp>
#include <core/memory.hpp>
#include <core/config.hpp>

struct Machine
{
	static inline constexpr auto kMemoryFlagsAll = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
	static inline constexpr auto kMemoryFlagsROM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagExecute;
	static inline constexpr auto kMemoryFlagsRAM = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagExecute;
	static inline constexpr auto kMemoryFlagsDevice = WHvMapGpaRangeFlagRead|WHvMapGpaRangeFlagWrite|WHvMapGpaRangeFlagTrackDirtyPages;


  Machine (Config const&);
  ~Machine ();

	Machine (Machine const&) = delete;
	auto operator = (Machine const&) -> Machine& = delete;
	Machine (Machine &&) = delete;
	auto operator = (Machine &&) -> Machine& = delete;

	auto MapMemory (std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t flags_v=kMemoryFlagsRAM, std::size_t offset_v=0u) -> void;	
	auto UnmapMemory (std::uint64_t base_v, std::uint64_t size_v) -> void;

	template <typename... T>
	auto InitializeMemory(T&&...args_v) -> std::size_t {
		auto const index_v = m_Memories.size();
		m_Memories.emplace_back(
			std::forward<T>(args_v)...);
		return index_v;
	}

private:
	std::vector<Memory> m_Memories; 
	WHV_PARTITION_HANDLE m_Partition{ nullptr };
};