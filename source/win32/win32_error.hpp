#pragma once

#include <source_location>
#include <string_view>
#include <stdexcept>
#include <string>

struct win32_error
	: public std::runtime_error
{
	win32_error(std::uint32_t errvalue_v = last_error(), 
		std::source_location location_v = std::source_location::current(),
		std::string_view code_v = "");

	inline auto errvalue() const -> std::uint32_t { return m_errvalue; }
	inline auto location() const -> std::source_location { return m_location; }
	
	static auto last_error() -> std::uint32_t;
	static auto to_string(std::uint32_t value_v) -> std::string;
	static auto assert(std::uint32_t errvalue_v, std::source_location location_v, std::string_view clode_line = "") -> void;

protected:
	std::uint32_t m_errvalue;
	std::source_location m_location;
};
