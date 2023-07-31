#include <win32/error.hpp>
#include <win32/workqueue.hpp>

#include <utility>
using win32::WorkInstance;
using win32::WorkQueue;
using win32::WorkItem;

using std::exchange;


////////////////////////////
//
//  WorkInstance
// 
////////////////////////////

WorkInstance::WorkInstance(PTP_CALLBACK_INSTANCE instance_v) noexcept
	: m_Handle(instance_v)
{}

auto WorkInstance::InitiateLongRunningTask() const -> bool
{
	return ::CallbackMayRunLong(m_Handle) ? true : false;
}

////////////////////////////
//
//  WorkItem
// 
////////////////////////////

#include <iostream>

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

auto WorkItem::Submit(WorkQueue& queue_v) -> void
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
	work_v.Submit(*this);
}

auto WorkQueue::Cbkenv() noexcept -> TP_CALLBACK_ENVIRON&
{
	return m_Cbkenv;
}

auto WorkQueue::Cbkenv() const noexcept -> TP_CALLBACK_ENVIRON const&
{
	return m_Cbkenv;
}
