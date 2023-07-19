#pragma once

#include <utils/bitmanip.hpp>
#include <core/constants.hpp>

#include <cstddef>
#include <cstdint>
#include <span>

namespace core
{
	struct Memory
	{		
		virtual ~Memory() = default;
		virtual auto Access(std::uint64_t address_v, bool write_v, std::uint8_t size_v, utils::bytes<8u> &data_v) -> std::uint32_t = 0;
		virtual auto Size () const noexcept -> std::size_t = 0u;
		virtual auto Data (std::size_t length_v=0u, std::size_t offset_v=0u) const noexcept -> std::span<std::byte const> = 0;
		virtual auto Data (std::size_t length_v=0u, std::size_t offset_v=0u) noexcept -> std::span<std::byte> = 0;
	};
}