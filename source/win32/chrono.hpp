#pragma once

#include <win32/windows.hpp>

#include <cstdint>
#include <chrono>

namespace win32
{
	struct filetime_clock
	{
		using rep = int64_t;
		using period = std::ratio<1, 10000000>;
		using duration = std::chrono::duration<rep, period>;
		using time_point = std::chrono::time_point<filetime_clock>;

		static auto now() noexcept -> time_point;

		static auto to_filetime(time_point const& tp) noexcept -> FILETIME;
		static auto from_filetime(FILETIME const& ft) noexcept -> time_point;
	};

  static inline auto query_performance_counter() -> uint64_t {
    LARGE_INTEGER value_v;
    if (::QueryPerformanceCounter(&value_v) == FALSE) {
      throw std::system_error{ static_cast<int>(::GetLastError()),
        std::system_category() };
    }
    return value_v.QuadPart;
  }

  static inline auto query_performance_frequency() -> uint64_t {
    LARGE_INTEGER value_v;
    if (::QueryPerformanceFrequency(&value_v) == FALSE) {
      throw std::system_error{ static_cast<int>(::GetLastError()),
        std::system_category() };
    }
    return value_v.QuadPart;
  }

}