#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <bitset>
#include <list>

#include <utils/smart_span.hpp>

#include <core/mapgparange.hpp>
#include <core/romimage.hpp>

namespace core
{
	struct Machine;
	struct Processor;
	struct Configuration;

	struct Memory
	{
		Memory (Machine&);

		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;

		auto FetchMemory(core::Processor const& vcpu_v, uint64_t address_v, uint64_t length_v, std::vector<std::byte>& output_v)->int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v)->int32_t;

	private:
		Machine& m_Machine;

		win32::unique_span<std::byte> m_MainMemory;
		std::list<MapGpaRange> m_MappedRanges;
		std::list<RomImage> m_MappedRoms;
		std::bitset<4096u> m_PageZeroStatus;

	};
}