#pragma once

#include <string>
#include <stdexcept>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

auto to_string(HRESULT result) -> std::string;

struct win32_error
	: public std::runtime_error
{
	win32_error(HRESULT hresult_v);
	inline auto value() const -> HRESULT { return m_value; }
protected:
	HRESULT m_value;
};
