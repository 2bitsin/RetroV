#include <win32/chrono.hpp>
#include <utils/algorithm.hpp>
#include <cassert>

auto win32::filetime_clock::now() noexcept -> time_point
{
	FILETIME filetime_v { };
	GetSystemTimePreciseAsFileTime(&filetime_v);
	return from_filetime(filetime_v);
}

auto win32::filetime_clock::to_filetime(time_point const& time_v) noexcept -> FILETIME
{
	FILETIME filetime_v{ };
	assert(time_v.time_since_epoch().count() >= 0);
	std::tie(filetime_v.dwHighDateTime, filetime_v.dwLowDateTime) =
		utils::integral_split_msw<uint32_t>(time_v.time_since_epoch().count());
	return filetime_v;
}

auto win32::filetime_clock::from_filetime(FILETIME const& filetime_v) noexcept -> time_point
{
	auto filetime_u64 = utils::integral_join_msw(filetime_v.dwHighDateTime, filetime_v.dwLowDateTime);
	assert(filetime_u64 < 0x8000000000000000ULL);
	return time_point{ duration{ (rep)filetime_u64 } };
}

