#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <span>

namespace utils 
{
	template <typename T>
	static inline auto take_span(std::span<T>& from_v, std::size_t size_v) -> std::span<T>
	{
		if (size_v < 1u || from_v.size() < 1u)
			return std::span<T>{};		
		size_v = std::min(size_v, from_v.size());
		auto shard_v = from_v.first(size_v);
		if (size_v < from_v.size()) {
			from_v = from_v.subspan(size_v);
		} else {
			from_v = std::span<T>{};
		}
		return shard_v;
	}

}