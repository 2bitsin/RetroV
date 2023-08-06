#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace device {

	struct font {

		static auto font_8x16 () -> std::span<std::byte const>;
	};

}