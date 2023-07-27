#pragma once

#include <cstdint>
#include <cstddef>

namespace utils {
	template <typename T>
	static inline constexpr auto crossover_mask(T const& lhs_v, T const& rhs_v, T const& mask_v) -> T {
		return (lhs_v&mask_v)|(rhs_v&~mask_v);
	}

	template <typename T>
	static inline constexpr auto crossover_bits(T const& lhs_v, T const& rhs_v, std::size_t count_v) -> T {
		auto const mask_v = (T(1u)<<count_v)-T(1u);
		return crossover_mask(lhs_v, rhs_v, mask_v);
	}

}