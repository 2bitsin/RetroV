#pragma once

#include <win32/windows.hpp>
#include <utils/paths.hpp>

#include <source_location>
#include <string_view>
#include <string>
#include <format>

namespace win32
{
	struct scope_name {

		scope_name(std::string_view description_v)
			: scope_name { make_wstring(description_v) }
		{}

		scope_name(std::wstring description_v)
			: m_handle { ::GetCurrentThread() }
			,	m_result { ::SetThreadDescription(
					m_handle, description_v.c_str()) == 0 }
		{}

		~scope_name() {
			if (m_result != S_OK) {
				::SetThreadDescription(m_handle, L"");
			}
		}

		scope_name(scope_name const&) = delete;
		scope_name& operator=(scope_name const&) = delete;

		scope_name(scope_name&&) = delete;
		scope_name& operator=(scope_name&&) = delete;

		scope_name(std::source_location source_location_v = std::source_location::current())
			: scope_name {  
					std::format (L"{} ({}:{})",
						make_wstring(source_location_v.function_name()),
						utils::make_relative_to_source_directory(source_location_v.file_name()).wstring(),
						source_location_v.line())
				}
			 
		{
		}

	public:
		static inline auto make_wstring(std::string_view source_v) -> std::wstring {
			return std::wstring{ source_v.begin(), source_v.end() };
		}

	private:
		HANDLE  m_handle;
		HRESULT m_result;
	};

}

