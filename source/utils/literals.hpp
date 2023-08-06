#pragma once

#include <cstdint>
#include <cstddef>

#include <stdexcept>

namespace size_literals
{
	static inline constexpr auto operator "" _paras(std::uint64_t const value_v) noexcept -> std::size_t {
		return value_v * 16u;
	}

	static inline constexpr auto operator "" _KiB(std::uint64_t const value_v) noexcept -> std::uint64_t {
		return std::uint64_t(value_v * 1024ull);
	}

	static inline constexpr auto operator "" _KiB(long double const value_v) noexcept -> std::uint64_t {
		return std::uint64_t(value_v * 1024.0);
	}

	static inline constexpr auto operator "" _pages(std::uint64_t const value_v) noexcept -> std::uint64_t {
		return operator ""_KiB (value_v * 4ull);
	}

	static inline constexpr auto operator "" _pages(long double const value_v) noexcept -> std::uint64_t {
		return operator ""_KiB (value_v * 4.0);
	}

	static inline constexpr auto operator "" _MiB(std::uint64_t const value_v) noexcept -> std::uint64_t {
		return operator ""_KiB (value_v * 1024ull) ;
	}

	static inline constexpr auto operator "" _MiB(long double const value_v) noexcept -> std::uint64_t {
		return operator ""_KiB (value_v * 1024.0);
	}

	static inline constexpr auto operator "" _GiB(std::uint64_t const value_v) noexcept -> std::uint64_t {
		return operator ""_MiB (value_v * 1024ull);
	}

	static inline constexpr auto operator "" _GiB(long double const value_v) noexcept -> std::uint64_t {
		return operator ""_MiB (value_v * 1024.0);
	}

	static inline constexpr auto operator "" _TiB(std::uint64_t const value_v) noexcept -> std::uint64_t {
		return operator ""_GiB (value_v * 1024ull);
	}

	static inline constexpr auto operator "" _TiB(long double const value_v) noexcept -> std::uint64_t {
		return operator ""_GiB (value_v * 1024.0);
	}
}

namespace misc_literals
{
	static inline constexpr auto operator "" _b (std::uint64_t const value_v) -> std::byte {
		if (value_v > 0xFFu) throw std::out_of_range("Byte value out of range.");
		return std::byte(value_v);
	}
}