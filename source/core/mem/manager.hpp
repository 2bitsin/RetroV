#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/as_bytes.hpp>

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <span>

namespace core
{
	struct Hypervisor;
}

namespace core::mem
{
	struct Manager 
	{
		static inline constexpr auto kMemoryFlagsAll = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsROM = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsRAM = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagExecute;
		static inline constexpr auto kMemoryFlagsDevice = WHvMapGpaRangeFlagRead | WHvMapGpaRangeFlagWrite | WHvMapGpaRangeFlagTrackDirtyPages;

		static inline constexpr auto kMaxGpaReadWriteSize = 16u;

		static inline constexpr auto kPhysicalAddress	= 0x0u;
		static inline constexpr auto kVirtualAddress = 0x1u;
		static inline constexpr auto kValidatedAddress = 0x2u;

		static inline constexpr auto kPageSize = 4096u;

		Manager(core::Hypervisor& hypervisor_v);

		Manager(Manager const&) = delete;
		Manager& operator=(Manager const&) = delete;

		Manager(Manager&&) noexcept ;
		auto operator=(Manager&&) noexcept -> Manager&;

		auto Swap (Manager& other_v) noexcept -> void;

		auto MapPhysical(std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v = 0u, 
			std::uint32_t flags_v = kMemoryFlagsRAM, std::uint64_t offset_v = 0u) -> void;

		auto UnmapPhysical(std::uint64_t base_v, std::uint64_t size_v = 0u) -> void;

		auto VirtualToPhysical(std::uint32_t index_v, std::uint64_t& inout_address_v, 
			WHV_TRANSLATE_GVA_FLAGS flags_v = WHvTranslateGvaFlagNone) const->WHV_TRANSLATE_GVA_RESULT_CODE;

		auto Write(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> data_v, 
			std::uint32_t flags_v = kVirtualAddress, WHV_CACHE_TYPE chache_v = WHvCacheTypeUncached) -> void;

		auto Fetch(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte      > data_v, 
			std::uint32_t flags_v = kVirtualAddress, WHV_CACHE_TYPE chache_v = WHvCacheTypeUncached) -> void;

		template <typename T> requires (std::is_trivial_v<T>) 
		auto FetchValue(std::uint32_t index_v, std::uint64_t address_v, std::uint32_t flags_v = kVirtualAddress,
			WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) -> T
		{
			T value_v { }; 
			Fetch(index_v, address_v, utils::as_mutable_bytes(value_v), flags_v, cache_v);
			return value_v;
		}

		template <typename... T> requires (sizeof...(T) > 1u && (std::is_trivial_v<T> && ...))
		auto FetchValue(std::uint32_t index_v, std::uint64_t address_v, std::uint32_t flags_v = kVirtualAddress,
			WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) -> std::tuple<T...>
		{
			return std::tuple{ FetchValue<T>(index_v, std::exchange(address_v, address_v + sizeof(T)), flags_v, cache_v)... };
		}


		template <typename T> requires (std::is_trivial_v<T>)
		auto WriteValue(std::uint32_t index_v, std::uint64_t address_v, T const& value_v, 
			std::uint32_t flags_v = kVirtualAddress, WHV_CACHE_TYPE cache_v = WHvCacheTypeUncached) -> void
		{
			Write(index_v, address_v, utils::as_bytes(value_v), flags_v, cache_v);			
		}

	protected:
		auto WriteSome(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> data_v, std::uint32_t flags_v = kVirtualAddress, WHV_CACHE_TYPE chache_v = WHvCacheTypeUncached) -> void;
		auto FetchSome(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte      > data_v, std::uint32_t flags_v = kVirtualAddress, WHV_CACHE_TYPE chache_v = WHvCacheTypeUncached) -> void;

	private:
		core::Hypervisor* m_Hypervisor;
		std::uint32_t m_LastID { 0u };
	};
}