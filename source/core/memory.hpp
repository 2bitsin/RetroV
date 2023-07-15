#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace core
{
	struct Memory
	{
		static inline constexpr const auto kPageSize = 4096u;
		virtual ~Memory() = default;
		virtual auto Access(std::uint64_t address_v, bool write_v, std::uint8_t size_v, std::uint8_t (&data_v) [8]) -> void = 0;
		virtual auto Size () const noexcept -> std::size_t = 0u;
		virtual auto Data (std::size_t length_v=0u, std::size_t offset_v=0u) const noexcept -> std::span<std::byte const> = 0;
		virtual auto Data (std::size_t length_v=0u, std::size_t offset_v=0u) noexcept -> std::span<std::byte> = 0;
	};
}