#pragma once

#include <cstddef>
#include <cstdint>

#include <utils/objects.hpp>
#include <core/pageblock.hpp>

namespace core
{
	struct MemoryController
		: public utils::uncopyable
		, public utils::unmovable
	{
		static inline constexpr const auto kPageSize = 0x1000u;
		static inline constexpr const auto kProtFetch = 0x1u;
		static inline constexpr const auto kProtWrite = 0x2u;
		static inline constexpr const auto kProtExecute = 0x4u;

		static inline constexpr const auto kProtRom = kProtFetch | kProtExecute;
		static inline constexpr const auto kProtRam = kProtFetch | kProtWrite | kProtExecute;
		static inline constexpr const auto kProtDev = kProtFetch | kProtWrite;

		static inline constexpr const auto kLastAddress = 0xFFFFFFFFFFFFFFFFull;
		static inline constexpr const auto kPageLimit = (kLastAddress >> 12u)+1u;

		MemoryController();

		auto MapRange(uint64_t address_base_v, uint32_t prot_mask_v, PageBlock block_v, uint64_t number_of_pages_v = );
		auto UnmapRange(uint64_t address_base_v, uint64_t number_of_pages_v = 0u);
		auto MapRange(uint64_t address_base_v, uint32_t prot_mask_v);


	};
}