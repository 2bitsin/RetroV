#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace device {

	auto GenericVGAFont8x16 () -> std::span<std::byte const>;

}