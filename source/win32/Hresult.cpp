#include "Hresult.hpp"

auto to_string(HRESULT result) -> std::string
{
	auto buffer_v{ (char*)nullptr };
	auto length_v{ ::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer_v, 0, NULL) };
	std::string string_v(buffer_v, length_v);
	::LocalFree(buffer_v);
	return string_v;
}


win32_error::win32_error(HRESULT hresult_v)
: std::runtime_error(to_string(hresult_v))
, m_value{ hresult_v }
{}
