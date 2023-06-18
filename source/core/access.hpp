#pragma once

#include <cstdint>
#include <cstddef>

namespace core
{
	namespace access
	{
		static inline constexpr auto const track_dirty = 0x08u;
		static inline constexpr auto const execute = 0x04u;
		static inline constexpr auto const write = 0x02u;
		static inline constexpr auto const read = 0x01u;

		static inline constexpr auto const all = read | write | execute;
		static inline constexpr auto const rom = read | execute;
		static inline constexpr auto const device = read | write;
	}
	auto protect_from_access(std::uint32_t access_v) -> std::uint32_t;
}