#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>

#include <win32/winhvpx.hpp>
#include <core/memory.hpp>
#include <core/config.hpp>


namespace core
{ 
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
	
		auto MapMemory (std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v=0u, std::uint32_t flags_v=kMemoryFlagsRAM, std::size_t offset_v=0u) -> void;	
		auto UnmapMemory (std::uint64_t base_v, std::uint64_t size_v=0u) -> void;
	
		template <typename... T>
		auto InitializeMemory(T&&...args_v) -> std::size_t {
			auto const index_v = m_Memories.size();
			m_Memories.emplace_back(
				std::forward<T>(args_v)...);
			return index_v;
		}

		auto InitializeProcessor(std::uint32_t index) -> void;

	protected:
		friend struct Config;

		auto InitializePartitionProperties() -> void;

		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void;

		template <typename T>
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T const& data_v) -> void {
			static_assert(std::is_trivially_copyable_v<T>);
			return SetProperty(code_v, &data_v, sizeof(T));
		}

		template <typename T>
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T& data_v) -> void {
			static_assert(std::is_trivially_copyable_v<T>);
			auto size_v = sizeof(T);
			return GetProperty(code_v, &data_v, size_v);
		}
	
	private:
		std::vector<Memory> m_Memories; 
		WHV_PARTITION_HANDLE m_Partition{ nullptr };
	};
}