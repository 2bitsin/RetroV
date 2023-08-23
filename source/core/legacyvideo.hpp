#pragma once

#include <cstdint>
#include <cstddef>

#include <list>

#include <utils/span.hpp>

#include <core/memory.hpp>

namespace core
{
	struct Machine;
	struct Processor;

	struct LegacyVideo
	{
		LegacyVideo(Machine& machine_v);

		auto Initialize() -> std::int32_t;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;

	private:
		Machine& m_Machine;
		std::list<Memory> m_VideoRAM;
	};
}