#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace device::video
{
	struct font 
	{
		std::uint8_t const cols;
		std::uint8_t const rows;
		std::uint8_t const stride;
		std::span<std::byte const> const data;
		static auto get_8x16 () -> font;
	};
}