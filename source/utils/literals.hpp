#pragma once

#include <cstdint>
#include <cstddef>

namespace size_literals
{

	static inline constexpr auto operator "" _KiB(std::uint64_t const value_v) noexcept -> std::size_t {
		return value_v * 1024u;
	}

	static inline constexpr auto operator "" _MiB(std::uint64_t const value_v) noexcept -> std::size_t {
		return operator ""_KiB (value_v) * 1024u;
	}

	static inline constexpr auto operator "" _GiB(std::uint64_t const value_v) noexcept -> std::size_t {
		return operator ""_MiB (value_v) * 1024u;
	}

	static inline constexpr auto operator "" _TiB(std::uint64_t const value_v) noexcept -> std::size_t {
		return operator ""_GiB (value_v) * 1024u;
	}


}