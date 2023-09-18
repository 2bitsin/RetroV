#include <win32/waitabletimer.hpp>
#include <win32/error.hpp>

using win32::waitable_timer;

waitable_timer::waitable_timer()
	: m_handle{ ::CreateWaitableTimerW(nullptr, FALSE, nullptr) }
{
	if (INVALID_HANDLE_VALUE==m_handle.get() || !m_handle)
		error::throw_last_error();
}

auto win32::waitable_timer::wait(milliseconds timeout_v, bool alertable_v) const -> bool 
{
again:
	switch(::WaitForSingleObjectEx(m_handle.get(), timeout_v.count(), alertable_v ? TRUE : FALSE))
	{
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
		return false;
	case WAIT_OBJECT_0:
		return true;
	case WAIT_IO_COMPLETION:
		goto again;
	default:
	case WAIT_FAILED:
		error::throw_last_error();
		break;		
	}
}

auto waitable_timer::set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, duration duetime_v, milliseconds period_v) -> void {
	assert(INVALID_HANDLE_VALUE != m_handle.get() && m_handle);
	LARGE_INTEGER duetime_lint{ .QuadPart = -duetime_v.count() };
	if (SetWaitableTimer(m_handle.get(), &duetime_lint,
		period_v.count(), callback_v, argument_v, FALSE))
		return;
	error::throw_last_error();
}

auto win32::waitable_timer::set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, time_point duetime_v, milliseconds period_v) -> void
{
	assert(INVALID_HANDLE_VALUE != m_handle.get() && m_handle);
	LARGE_INTEGER duetime_lint{ .QuadPart = duetime_v.time_since_epoch().count() };
	if (SetWaitableTimer(m_handle.get(), &duetime_lint,
		period_v.count(), callback_v, argument_v, FALSE))
		return;
	error::throw_last_error();
}


