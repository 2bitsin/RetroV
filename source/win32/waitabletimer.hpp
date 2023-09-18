#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/filetime_clock.hpp>

#include <functional>
#include <cassert>
#include <chrono>
#include <memory>

namespace win32
{ 

	struct WaitableTimer
	{

		struct close_handle
		{
			auto operator () (void* handle_v) noexcept -> void {				
				if (handle_v&&INVALID_HANDLE_VALUE!=handle_v) 
					::CloseHandle(handle_v);
			}
		};

		using time_point = win32::filetime_clock::time_point;
		using hunred_nanoseconds = win32::filetime_clock::duration;
		using milliseconds = std::chrono::duration<int32_t, std::milli>;
		using unique_handle = std::unique_ptr<void, close_handle>;

		WaitableTimer();

		~WaitableTimer() = default;

		WaitableTimer(const WaitableTimer&) = delete;
		auto operator = (const WaitableTimer&) -> WaitableTimer& = delete;

		WaitableTimer(WaitableTimer&&) = default;
		auto operator = (WaitableTimer&&) -> WaitableTimer& = default;

		template <typename Callee, typename DueTime>
		auto Wait(Callee&& callee, DueTime&& duetime_v, milliseconds period_v=milliseconds::zero()) -> void {
			using namespace std::chrono;
			static constexpr auto const proxyfun_s = [](void* this_v, unsigned long timelo_v, unsigned long timehi_v) -> void {				
				auto time_v = filetime_clock::from_filetime({ timelo_v, timehi_v });
				static_cast<WaitableTimer*>(this_v)->m_Callee(time_v); 
			};
			m_Callee = std::forward<Callee>(callee);
			InternalWait(proxyfun_s, this, duration_cast<hunred_nanoseconds>(duetime_v), period_v);
		}


	private:
		auto InternalWait(PTIMERAPCROUTINE callback_v, void* argument_v, hunred_nanoseconds duetime_v, milliseconds period_v = milliseconds::zero()) -> void;

	private:
		unique_handle m_Handle;
		std::function<void(time_point)> m_Callee;
	};
}