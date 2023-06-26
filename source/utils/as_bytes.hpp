#pragma once

#include <span>
#include <cstddef>
#include <cstdint>

namespace utils {
	template <typename T> 
	requires (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>)
	static inline auto as_bytes (T const& value) noexcept -> std::span<std::byte const> {
		return {reinterpret_cast<std::byte const*>(&value), sizeof(T)};
	}

	template <typename T>
	requires (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>)
	static inline auto as_mutable_bytes (T& value) noexcept -> std::span<std::byte> {
		return {reinterpret_cast<std::byte*>(&value), sizeof(T)};
	}
}