#pragma once

#include <cstdint>
#include <cstddef>

#include <utils/span.hpp>

namespace core
{
	struct Machine;

	struct LegacyVideo
	{
		LegacyVideo(Machine& machine_v);

		auto IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;

	private:
		Machine& m_Machine;
	};
}