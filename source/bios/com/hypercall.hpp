#pragma once

#include <cstdint>
#include <cstddef>

namespace core::hypercall
{
  #define HYPERCALL_DEFINE_MAJOR(X, Y, Z) static inline constexpr const uint8_t X = Y;
  #define HYPERCALL_DEFINE(X, Y, Z) static inline constexpr const uint16_t X = (Y << 8) | Z;
	#define CONSTANT2_DEFINE(X, Y) static inline constexpr const uint16_t X = Y;
	#define CONSTANT4_DEFINE(X, Y) static inline constexpr const uint32_t X = Y;
  #include "hypercall.h"
  #undef HYPERCALL_DEFINE_MAJOR
  #undef HYPERCALL_DEFINE
	#undef CONSTANT2_DEFINE
	#undef CONSTANT4_DEFINE
}