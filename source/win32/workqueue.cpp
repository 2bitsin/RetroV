#include <win32/error.hpp>
#include <win32/workqueue.hpp>

#include <utility>
using win32::WorkInstance;
using win32::WorkItem;
using win32::WorkTimer;
using win32::WorkQueue;

using std::exchange;


////////////////////////////
//
//  WorkInstance
// 
////////////////////////////

WorkInstance::WorkInstance(PTP_CALLBACK_INSTANCE instance_v) noexcept
	: m_Handle(instance_v)
{}

auto WorkInstance::MayRunLong() const -> bool
{
	return ::CallbackMayRunLong(m_Handle) ? true : false;
}

////////////////////////////
//
//  WorkItem
// 
////////////////////////////

WorkItem::~WorkItem() {
	if (nullptr != m_Handle) {	
		::WaitForThreadpoolWorkCallbacks(m_Handle, TRUE);
		::CloseThreadpoolWork(m_Handle);
	}
}

WorkItem::WorkItem(WorkItem&& from_v) noexcept
	: m_Cbkfun(exchange(from_v.m_Cbkfun, nullptr))
	, m_Handle(exchange(from_v.m_Handle, nullptr))
{}

auto WorkItem::operator=(WorkItem&& from_v) noexcept -> WorkItem& {
	if (this != &from_v) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WorkItem::swap(WorkItem& with_v) noexcept -> void
{
	std::swap(m_Cbkfun, with_v.m_Cbkfun);
	std::swap(m_Handle, with_v.m_Handle);
}

auto WorkItem::Handle() const noexcept -> PTP_WORK
{
	return m_Handle;
}

auto WorkItem::Wait() const -> void
{
	::WaitForThreadpoolWorkCallbacks(m_Handle, FALSE);
}

auto WorkItem::Cancel() const -> void
{
	::WaitForThreadpoolWorkCallbacks(m_Handle, TRUE);
}

auto WorkItem::SubmitTo(WorkQueue& queue_v) -> void
{
	if (nullptr != m_Handle) { 
		::CloseThreadpoolWork(m_Handle);
	}

	m_Handle = ::CreateThreadpoolWork(&EntryPoint, this, &queue_v.Cbkenv());

	if (nullptr != m_Handle) {
		return ::SubmitThreadpoolWork(m_Handle);
	}

	throw error(error::last_error());	
}

void WorkItem::EntryPoint(PTP_CALLBACK_INSTANCE instance_v, void* context_v, PTP_WORK work_v)
{
	auto const work_ptr = static_cast<WorkItem*>(context_v);
	work_ptr->m_Cbkfun(WorkInstance(instance_v), *work_ptr);
}

////////////////////////////
//
//  WorkTimer
// 
////////////////////////////

WorkTimer::~WorkTimer() {
	Cancel();
}

WorkTimer::WorkTimer(WorkTimer&& from_v) noexcept
	: m_Cbkfun(exchange(from_v.m_Cbkfun, {}))
	, m_Handle(exchange(from_v.m_Handle, nullptr))
{}

auto WorkTimer::operator=(WorkTimer&& from_v) noexcept -> WorkTimer& {
	if (this != &from_v) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WorkTimer::swap(WorkTimer& with_v) -> void {
	std::swap(m_Cbkfun, with_v.m_Cbkfun);
	std::swap(m_Handle, with_v.m_Handle);
}

auto WorkTimer::Handle() const noexcept -> PTP_TIMER {
	return m_Handle;
}

auto WorkTimer::Cancel() -> void {
	if (nullptr != m_Handle) {
		::WaitForThreadpoolTimerCallbacks(m_Handle, TRUE);
		::CloseThreadpoolTimer(m_Handle);
		m_Handle = nullptr;
	}
}

auto WorkTimer::ToFileTime(std::chrono::system_clock::time_point time_v) -> FILETIME {
	using namespace std::chrono;

  auto time_tv = system_clock::to_time_t(time_v);

	ULARGE_INTEGER time_value {
		.QuadPart = (time_tv * 10000000ULL) + 116444736000000000ULL
	};

	return FILETIME {
		.dwLowDateTime  = time_value.LowPart,
		.dwHighDateTime = time_value.HighPart
	};
}

auto WorkTimer::SubmitTo(WorkQueue& queue_v, FILETIME expire_v, std::uint32_t period_millisec_v) -> void
{
	if (nullptr != m_Handle) {
		::CloseThreadpoolTimer(m_Handle);
	}

	m_Handle = ::CreateThreadpoolTimer(&EntryPoint, this, &queue_v.Cbkenv());

	if (nullptr != m_Handle) {		
		return ::SetThreadpoolTimer(m_Handle, &expire_v, period_millisec_v, 0);
	}

	throw error(error::last_error());
}

auto WorkTimer::EntryPoint(PTP_CALLBACK_INSTANCE instance_v, void* context_v, PTP_TIMER timer_v) -> void
{
	auto const timer_ptr = static_cast<WorkTimer*>(context_v);
	timer_ptr->m_Cbkfun(WorkInstance(instance_v), *timer_ptr);
}

auto WorkTimer::SubmitTo(WorkQueue& queue_v, time_point_type expire_v, duration_type period_v) -> void
{
	using namespace std::chrono;
	auto const expire_filetime_v = ToFileTime(expire_v);
	auto const period_millisec_v = duration_cast<milliseconds>(period_v).count();
	if (period_millisec_v > 0xFFFFFFFFu) throw std::invalid_argument("period");
	return SubmitTo(queue_v, expire_filetime_v, period_millisec_v & 0xFFFFFFFFu);
}

auto WorkTimer::SubmitTo(WorkQueue& queue_v, duration_type expire_v, duration_type period_v) -> void
{
	using namespace std::chrono;
	auto const expire_100nanos_v = -duration_cast<duration<std::uint64_t, std::ratio<1, 10000000>>>(expire_v).count();
	auto const period_millisec_v = duration_cast<milliseconds>(period_v).count();
	if (period_millisec_v > 0xFFFFFFFFu) throw std::invalid_argument("period");
	FILETIME expire_filetime_v{
		.dwLowDateTime = (std::uint32_t)(expire_100nanos_v & 0xFFFFFFFFu),
		.dwHighDateTime = (std::uint32_t)(expire_100nanos_v >> 32)
	};
	return SubmitTo(queue_v, expire_filetime_v, period_millisec_v & 0xFFFFFFFFu);
}

////////////////////////////
//
//  WorkQueue
// 
////////////////////////////

WorkQueue::WorkQueue() 
	: m_Handle(::CreateThreadpool(nullptr))
{
	if(nullptr == m_Handle) { 
		throw error(error::last_error());
	}
	::InitializeThreadpoolEnvironment(&m_Cbkenv);
	::SetThreadpoolCallbackPool(&m_Cbkenv, m_Handle);
}

WorkQueue::~WorkQueue() {
	if (nullptr != m_Handle) {
		::CloseThreadpool(m_Handle);
	}
}

WorkQueue::WorkQueue(WorkQueue&& from_v) noexcept
	: m_Handle(exchange(from_v.m_Handle, nullptr))
	, m_Cbkenv(exchange(from_v.m_Cbkenv, TP_CALLBACK_ENVIRON{}))
{}

auto WorkQueue::operator=(WorkQueue&& from_v) noexcept -> WorkQueue& {
	if (this != &from_v) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WorkQueue::Handle() const noexcept -> PTP_POOL
{
	return m_Handle;
}

auto WorkQueue::swap(WorkQueue& from_v) -> void	
{
	std::swap(m_Handle, from_v.m_Handle);
	std::swap(m_Cbkenv, from_v.m_Cbkenv);
}

auto WorkQueue::Submit(WorkItem& work_v) -> void
{
	work_v.SubmitTo(*this);
}

auto WorkQueue::Cbkenv() noexcept -> TP_CALLBACK_ENVIRON&
{
	return m_Cbkenv;
}

auto WorkQueue::Cbkenv() const noexcept -> TP_CALLBACK_ENVIRON const&
{
	return m_Cbkenv;
}
