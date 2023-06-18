#include <win32/win32_error.hpp>
#include <win32/windows.hpp>

auto win32_error::last_error() -> std::uint32_t
{
	return ::GetLastError();
}

auto win32_error::to_string(std::uint32_t result) -> std::string
{
	auto buffer_v{ (char*)nullptr };
	auto length_v{ ::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&buffer_v, 0, NULL) };
	std::string string_v(buffer_v, length_v);
	::LocalFree(buffer_v);
	return string_v;
}

auto win32_error::assert(std::uint32_t errvalue_v, std::source_location location_v, std::string_view code_v) -> void
{
	if (errvalue_v != ERROR_SUCCESS) {		
		throw win32_error(errvalue_v, std::move(location_v), code_v);
	}
}

using namespace std::string_literals;
win32_error::win32_error(std::uint32_t errvalue_v, std::source_location location_v, std::string_view code_v)
: std::runtime_error(std::string(code_v) + ":"s + to_string(errvalue_v))
, m_errvalue { errvalue_v }
, m_location { std::move(location_v) }
{}
