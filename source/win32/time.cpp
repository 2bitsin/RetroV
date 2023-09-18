#include <win32/time.hpp>

auto win32::TimePointToFileTime(std::chrono::system_clock::time_point time_v) -> FILETIME 
{
	using namespace std::chrono;
	auto time_tv = system_clock::to_time_t(time_v);
	ULARGE_INTEGER time_value{
		.QuadPart = (time_tv * 10000000ULL) + 116444736000000000ULL
	};
	return FILETIME{
		.dwLowDateTime = time_value.LowPart,
		.dwHighDateTime = time_value.HighPart
	};
}