#include <win32/error.hpp>
#include <win32/windows.hpp>

#include <format>

#include <utils/logger.hpp>

using win32::error;

auto error::last_error() -> std::int32_t
{
	return ::GetLastError();
}

auto error::to_string(std::int32_t result) -> std::string
{
	auto buffer_v{ (char*)nullptr };
	auto length_v{ ::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&buffer_v, 0, NULL) };
	std::string string_v(buffer_v, length_v);
	::LocalFree(buffer_v);
	return string_v;
}

auto error::__assert__(std::int32_t errvalue_v, std::source_location location_v, std::string_view code_v) -> void
{
	if (errvalue_v != ERROR_SUCCESS) {		
		throw error(errvalue_v, code_v, std::move(location_v));
	}
}

auto error::__notify__(std::int32_t errvalue_v, std::source_location location_v, std::string_view code_line) -> void
{
	using utils::logger;
	if (errvalue_v != ERROR_SUCCESS) {
		logger::error(logger::deflog, "{} failed : {} ({}:{}:{})", code_line, to_string(errvalue_v), 
			location_v.file_name(), location_v.line(), location_v.column());
	}
}

auto error::throw_last_error(std::source_location location_v) -> void
{
	throw error(last_error(), "", std::move(location_v));
}

using namespace std::string_literals;
error::error(std::int32_t errvalue_v, std::string_view code_v, std::source_location location_v)
	: std::runtime_error(std::format("{}:{}:{}: {}",  		
		location_v.file_name(), 
		location_v.line(), 
		location_v.column(),
		to_string(errvalue_v)))
	, m_errvalue { errvalue_v }
	, m_location { std::move(location_v) }
{}
