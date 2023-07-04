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

	template <typename T>
		requires (std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>)
	static inline auto as_bytes(T const& value) noexcept -> std::span<std::byte const> {
		return { reinterpret_cast<std::byte const*>(&value), sizeof(T) };
	}

	template <typename T>
		requires (std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>)
	static inline auto as_mutable_bytes(T& value) noexcept -> std::span<std::byte> {
		return { reinterpret_cast<std::byte*>(&value), sizeof(T) };
	}

	template <typename T, typename Q>
		requires (std::is_trivial_v<Q>&& std::is_trivial_v<T>)
	static inline auto mutable_span_as(std::span<Q> input_v) -> std::span<T> {
		return { reinterpret_cast<T*>(input_v.data()), input_v.size_bytes() / sizeof(T) };
	}

	template <typename T, typename Q>
		requires (std::is_trivial_v<Q>&& std::is_trivial_v<T>)
	static inline auto span_as(std::span<Q const> input_v) -> std::span<T const> {
		return { reinterpret_cast<T const*>(input_v.data()), input_v.size_bytes() / sizeof(T) };
	}
}