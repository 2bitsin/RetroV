#include <win32/waitabletimer.hpp>
#include <win32/error.hpp>

using win32::waitable_timer;

waitable_timer::waitable_timer(uint32_t flags_v)
	: m_timer 
	{	::CreateWaitableTimerExW(nullptr, nullptr, 
			(flags_v&high_resolution_flag?CREATE_WAITABLE_TIMER_HIGH_RESOLUTION:0)|
			(flags_v&manual_reset_flag?CREATE_WAITABLE_TIMER_MANUAL_RESET:0),
		TIMER_ALL_ACCESS) }
  , m_event
  { ::CreateEventW(nullptr, FALSE, FALSE, nullptr) }

{
	if (INVALID_HANDLE_VALUE==m_timer.get() || !m_timer) error::throw_last_error();
  if (INVALID_HANDLE_VALUE==m_event.get() || !m_event) error::throw_last_error();
}

auto win32::waitable_timer::wait(milliseconds timeout_v, uint32_t flags_v) const -> wait_status 
{
  void* handles_v[] = { m_timer.get(), m_event.get() };
  auto const wait_result_v = ::WaitForMultipleObjectsEx(
    2u, handles_v, FALSE, timeout_v.count(), 
    (flags_v & wait_alertable) ? TRUE : FALSE);

	while(true)
	switch(wait_result_v)
	{
	case WAIT_TIMEOUT:
		return wait_timedout;
	case WAIT_OBJECT_0:
		return timer_elapsed;
  case WAIT_OBJECT_0+1:
    return wait_cancelled;
	case WAIT_IO_COMPLETION:
    if (flags_v & wait_cancel_after_apc) 
      return wait_cancelled_by_apc;		
    continue;
  case WAIT_ABANDONED:
    throw std::logic_error(__FUNCTION__ " : WAIT_ABANDONED");
  case WAIT_FAILED:
	default:
		error::throw_last_error();
		break;		
	}
}

auto waitable_timer::wait(uint32_t flags_v) const -> wait_status
{
	return wait(milliseconds(INFINITE), flags_v);
}

auto waitable_timer::abort() const -> void
{
	if(!::CancelWaitableTimer(m_timer.get()))
		error::throw_last_error();
}

auto waitable_timer::reset() const -> void
{
	if(!::ResetEvent(m_timer.get()))
		error::throw_last_error();
}

auto waitable_timer::cancel_wait() const -> void
{
  if(!::SetEvent(m_event.get()))
    error::throw_last_error();
}

auto waitable_timer::set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, duration duetime_v, milliseconds period_v) -> void {
	assert(INVALID_HANDLE_VALUE != m_timer.get() && m_timer);
	LARGE_INTEGER duetime_lint{ .QuadPart = -duetime_v.count() };
	if (SetWaitableTimer(m_timer.get(), &duetime_lint,
		period_v.count(), callback_v, argument_v, FALSE))
		return;
	error::throw_last_error();
}

auto win32::waitable_timer::set_raw(PTIMERAPCROUTINE callback_v, void* argument_v, time_point duetime_v, milliseconds period_v) -> void
{
	assert(INVALID_HANDLE_VALUE != m_timer.get() && m_timer);
	LARGE_INTEGER duetime_lint{ .QuadPart = duetime_v.time_since_epoch().count() };
	if (SetWaitableTimer(m_timer.get(), &duetime_lint,
		period_v.count(), callback_v, argument_v, FALSE))
		return;
	error::throw_last_error();
}


