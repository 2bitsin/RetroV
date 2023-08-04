#pragma once

#include <source_location>
#include <string_view>
#include <stdexcept>
#include <string>

namespace win32
{
	struct error
		: public std::runtime_error
	{
		error(std::int32_t errvalue_v = last_error(), std::string_view code_v = "",
			std::source_location location_v = std::source_location::current());
	
		inline auto errvalue() const -> std::int32_t { return m_errvalue; }
		inline auto location() const -> std::source_location { return m_location; }
		
		static auto last_error() -> std::int32_t;
		static auto to_string(std::int32_t value_v) -> std::string;
		static auto __assert__(std::int32_t errvalue_v, std::source_location location_v, std::string_view clode_line = "") -> void;
	
		static auto throw_last_error (std::source_location location_v = std::source_location::current()) -> void;

	protected:
		std::int32_t m_errvalue;
		std::source_location m_location;
	};	


	struct error_deferred {

		inline error_deferred (std::int32_t value_v = 0u, std::source_location sloc_v = 
			std::source_location::current())
			: m_value(value_v)
			, m_srcloc(sloc_v)
		{}
		
		inline ~error_deferred() noexcept(false) {
			rethrow_error();
		}

		inline auto drop_error() {
			m_value = 0u;
		}

		inline auto rethrow_error() -> void {			
			using namespace std;
			if (0u==uncaught_exceptions() && 0u!=m_value) {
				throw error(exchange(m_value, 0u), "", m_srcloc);
			}
		}

		inline auto get_error() const -> std::uint32_t {
			return m_value;
		}

	private:
		std::int32_t m_value;
		std::source_location m_srcloc;
	};

}
#define WIN32_ERROR_ASSERT(expression) ::win32::error::__assert__(expression, std::source_location::current(), #expression)
