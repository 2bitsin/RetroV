#include <win32/error.hpp>
#include <win32/workqueue.hpp>

#include <utility>
#include "workqueue.hpp"

using win32::WorkQueue;
using win32::Work;


WorkQueue::WorkQueue() 
	: m_Handle(::CreateThreadpool(nullptr))
{
	if(nullptr == m_Handle) { 
		throw error(win32::error::last_error());
	}
	::InitializeThreadpoolEnvironment(&m_Cbkenv);
	::SetThreadpoolCallbackPool(&m_Cbkenv, m_Handle);
}

WorkQueue::~WorkQueue() {
	if (nullptr != m_Handle) {
		::CloseThreadpool(m_Handle);
	}
}

using std::exchange;
WorkQueue::WorkQueue(WorkQueue&& from_v) 
	: m_Handle(exchange(from_v.m_Handle, nullptr))
	, m_Cbkenv(exchange(from_v.m_Cbkenv, TP_CALLBACK_ENVIRON{}))
{}

auto WorkQueue::operator=(WorkQueue&& from_v) -> WorkQueue& {
	if (this != &from_v) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WorkQueue::Submit(Work) -> void
{
}

auto WorkQueue::Handle() const -> TP_POOL*
{
	return m_Handle;
}

auto WorkQueue::swap(WorkQueue& from_v) -> void	
{
	std::swap(m_Handle, from_v.m_Handle);
	std::swap(m_Cbkenv, from_v.m_Cbkenv);
}

auto WorkQueue::Cbkenv() -> TP_CALLBACK_ENVIRON&
{
	return m_Cbkenv;
}

