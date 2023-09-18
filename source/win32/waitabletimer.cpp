#include <win32/waitabletimer.hpp>
#include <win32/error.hpp>

using win32::waitable_timer;

waitable_timer::waitable_timer(std::uint32_t flags_v)
	: m_handle 
	{	::CreateWaitableTimerExW(nullptr, nullptr, 
			(flags_v&high_resolution_flag?CREATE_WAITABLE_TIMER_HIGH_RESOLUTION:0)|
			(flags_v&manual_reset_flag?CREATE_WAITABLE_TIMER_MANUAL_RESET:0),
		TIMER_ALL_ACCESS) }
{
	if (INVALID_HANDLE_VALUE==m_handle.get() || !m_handle)
		error::throw_last_error();
}

auto win32::waitable_timer::wait(milliseconds timeout_v, bool alertable_v) const -> bool 
{
	while(true)
	switch(::WaitForSingleObjectEx(m_handle.get(), timeout_v.count(), 
		alertable_v ? TRUE : FALSE))
	{
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
		return false;
	case WAIT_OBJECT_0:
		return true;
	case WAIT_IO_COMPLETION:
		continue;
	default:
	case WAIT_FAILED:
		error::throw_last_error();
		break;		
	}
}

auto win32::waitable_timer::reset() const -> void
{
	if(!::ResetEvent(m_handle.get()))
		error::throw_last_error();
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


