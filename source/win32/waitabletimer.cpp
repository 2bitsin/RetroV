#include <win32/waitabletimer.hpp>
#include <win32/error.hpp>

using win32::WaitableTimer;

WaitableTimer::WaitableTimer()
	: m_Handle{ ::CreateWaitableTimerW(nullptr, FALSE, nullptr) }
{
	if (INVALID_HANDLE_VALUE==m_Handle.get() || !m_Handle)
		error::throw_last_error();
}

auto WaitableTimer::InternalWait(PTIMERAPCROUTINE callback_v, void* argument_v, hunred_nanoseconds duetime_v, milliseconds period_v) -> void {
	assert(INVALID_HANDLE_VALUE != m_Handle.get() && m_Handle);
	LARGE_INTEGER duetime_lint{ .QuadPart = -duetime_v.count() };
	if (SetWaitableTimer(m_Handle.get(), &duetime_lint,
		period_v.count(), callback_v, argument_v, FALSE))
		return;
	error::throw_last_error();
}


