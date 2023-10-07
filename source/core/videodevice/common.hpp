#pragma once

#include <cstdint>
#include <cstddef>
#include <chrono>

#include <win32/memory.hpp>
#include <win32/chrono.hpp>

#include <utils/smart_span.hpp>
#include <utils/span.hpp>

namespace core::videodevice
{
	using duration_type = win32::filetime_clock::duration;
	using buffer_type = win32::unique_span<std::byte>;

}