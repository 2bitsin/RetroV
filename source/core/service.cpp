#include <core/scheduler.hpp>
#include <core/service.hpp>

#include <cassert>

using core::Service;

Service::Service() noexcept
	: m_Scheduler(nullptr)
	, m_DispatchTbl(nullptr)
{}

Service::Service(Scheduler& scheduler_v, std::function<task_type> callback_v)
	: m_Scheduler(&scheduler_v)
	, m_DispatchTbl(std::make_unique<state_type>())	
{
	auto& state_v{ *m_DispatchTbl };
	state_v.m_DispatchTbl = kNotStarted;
	state_v.m_StopSource = std::stop_source{};
	state_v.m_StopTarget = state_v.m_StopSource.get_token();
	state_v.m_Thread = std::jthread{[this,
		callback_v = std::move(callback_v)] {
		return callback_v(GetScheduler(), *this);
	}};
}

Service::Service(Service&& prev_v) noexcept
	: m_Scheduler(std::exchange(prev_v.m_Scheduler, nullptr))
	, m_DispatchTbl(std::move(prev_v.m_DispatchTbl))
{}

auto Service::operator=(Service&& prev_v) noexcept -> Service& {
	if (this==&prev_v)
		return *this;
	auto tmp_v{ std::move(prev_v) };
	tmp_v.Swap(*this); 
	return *this;
}

Service::~Service() {
	Stop();
}

auto Service::Swap(Service& prev_v) noexcept -> void {
	std::swap(m_Scheduler, prev_v.m_Scheduler);
	std::swap(m_DispatchTbl, prev_v.m_DispatchTbl);
}

auto Service::Pause() -> void {
	assert(m_DispatchTbl);
	auto& state_v{ *m_DispatchTbl };
	if (state_v.m_Waiting) return;
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_DispatchTbl = kPaused;
	state_v.m_CondVar.wait(lock_v, [this, &state_v]()->bool{
		return kPaused==state_v.m_DispatchTbl
		  	|| state_v.m_StopTarget.stop_requested();		
	});
}

auto Service::Stop() -> void {	
	if (!m_DispatchTbl) return;
	auto& state_v{ *m_DispatchTbl };
	state_v.m_StopSource.request_stop();		
	state_v.m_CondVar.notify_all();
	state_v.m_Thread.join();	
	m_DispatchTbl.reset();
} 
auto Service::Resume() -> void {
	assert(m_DispatchTbl);
	auto& state_v{ *m_DispatchTbl };
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_DispatchTbl = kRunning;
	state_v.m_CondVar.notify_all();	
}

auto Service::StopRequested() -> bool {
	assert(m_DispatchTbl);
	Yield();
	auto& state_v{ *m_DispatchTbl };
	std::unique_lock lock_v{ state_v.m_Mutex };
	auto result_v{ state_v.m_StopTarget.stop_requested() };
	if (result_v) state_v.m_DispatchTbl = kStopped;
	return result_v;
}

auto Service::Yield() -> void {
	assert(m_DispatchTbl);
	auto& state_v{ *m_DispatchTbl };
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_CondVar.notify_one();
	state_v.m_Waiting = true;
	state_v.m_CondVar.wait(lock_v, [this, &state_v]()->bool{
		return kRunning==state_v.m_DispatchTbl 
			  || state_v.m_StopTarget.stop_requested(); 
	});
	state_v.m_Waiting = false;
}

auto Service::GetScheduler() const -> Scheduler& {
	return *m_Scheduler;
}
