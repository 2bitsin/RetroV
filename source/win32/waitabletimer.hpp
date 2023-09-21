#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/chrono.hpp>

#include <functional>
#include <cassert>
#include <chrono>
#include <memory>

namespace win32
{ 

	struct waitable_timer
	{
		struct close_handle
		{
			auto operator () (void* handle_v) noexcept -> void {				
				if (handle_v&&INVALID_HANDLE_VALUE!=handle_v) 
					::CloseHandle(handle_v);
			}
		};

		using time_point = win32::filetime_clock::time_point;
		using duration = win32::filetime_clock::duration;
		using milliseconds = std::chrono::duration<int32_t, std::milli>;
		using unique_handle = std::unique_ptr<void, close_handle>;

		static inline constexpr const auto manual_reset_flag = 1u;
		static inline constexpr const auto high_resolution_flag = 2u;

		waitable_timer(std::uint32_t flags_v = 0u);
		~waitable_timer() = default;

		waitable_timer(const waitable_timer&) = delete;
		auto operator = (const waitable_timer&) -> waitable_timer& = delete;

		waitable_timer(waitable_timer&&) = default;
		auto operator = (waitable_timer&&) -> waitable_timer& = default;

		template <typename Callee, typename Rep, typename Period>
		requires std::is_invocable_v<Callee, time_point>
		inline auto set(Callee&& callee, std::chrono::duration<Rep, Period> duetime_v, milliseconds period_v=milliseconds::zero()) -> waitable_timer& {
			using namespace std::chrono;
			static constexpr auto const proxyfun_s = [](void* this_v, auto... time_v) -> void {				
				auto const filetime_v = filetime_clock::from_filetime({ time_v... });
				assert(nullptr!=this_v);
				static_cast<waitable_timer*>(this_v)->m_callee(filetime_v); 
			};
			m_callee = std::forward<Callee>(callee);
			set_raw(proxyfun_s, this, duration_cast<duration>(duetime_v), period_v);
			return *this;
		}

		template <typename Callee>
		requires std::is_invocable_v<Callee, time_point>
		inline auto set(Callee&& callee, time_point duetime_v, milliseconds period_v = milliseconds::zero()) -> waitable_timer& {
			using namespace std::chrono;
			static constexpr auto const proxyfun_s = [](void* this_v, auto... time_v) -> void {
				auto const filetime_v = filetime_clock::from_filetime({ time_v... });
				assert(nullptr != this_v);
				static_cast<waitable_timer*>(this_v)->m_callee(filetime_v);
				};
			m_callee = std::forward<Callee>(callee);
			set_raw(proxyfun_s, this, duetime_v, period_v);
			return *this;
		}
		

		template <typename Rep, typename Period>
		inline auto set(std::chrono::duration<Rep, Period> duetime_v, milliseconds period_v = milliseconds::zero()) -> waitable_timer& {
			set_raw(nullptr, nullptr, duration_cast<duration>(duetime_v), period_v);
			return *this;
		}

		inline auto set(time_point duetime_v, milliseconds period_v = milliseconds::zero()) -> waitable_timer& {
			set_raw(nullptr, nullptr, duetime_v, period_v);
			return *this;
		}

		auto wait(milliseconds timeout_v, bool alertable_v=true) const -> bool;
		auto wait(bool alertable_v=true) const -> bool;
		auto abort() const -> void;
		auto reset() const -> void;

	private:
		auto set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, duration duetime_v, milliseconds period_v = milliseconds::zero()) -> void;
		auto set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, time_point duetime_v, milliseconds period_v = milliseconds::zero()) -> void;

	private:
		unique_handle m_handle;
		std::function<void(time_point)> m_callee;
	};
}