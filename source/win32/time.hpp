#pragma once

#include <win32/windows.hpp>

#include <chrono>

namespace win32
{
	auto TimePointToFileTime(std::chrono::system_clock::time_point time_v) -> FILETIME;
}