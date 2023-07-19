#pragma once

#include <cstdint>
#include <cstddef>

namespace core
{
	static inline constexpr std::size_t kPageSize = 4096u;
	static inline constexpr const auto kLastAddress = 0xFFFFFFFFFFFFFFFFull;
	static inline constexpr const auto kPageLimit = (kLastAddress >> 12u) + 1u;
}