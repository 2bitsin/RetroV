#include <core/scheduler.hpp>
#include <core/service.hpp>

#include <cassert>

using core::Service;

Service::Service() noexcept
	: m_Scheduler(nullptr)
	, m_State(nullptr)
{}

Service::Service(Scheduler& scheduler_v, std::function<task_type> callback_v)
	: m_Scheduler(&scheduler_v)
	, m_State(std::make_unique<state_type>())	
{
	auto& state_v{ *m_State };
	state_v.m_State = kNotStarted;
	state_v.m_StopSource = std::stop_source{};
	state_v.m_StopTarget = state_v.m_StopSource.get_token();
	state_v.m_Thread = std::jthread{[this,
		callback_v = std::move(callback_v)] {
		return callback_v(GetScheduler(), *this);
	}};
}

Service::Service(Service&& prev_v) noexcept
	: m_Scheduler(std::exchange(prev_v.m_Scheduler, nullptr))
	, m_State(std::move(prev_v.m_State))
{}

auto Service::operator=(Service&& prev_v) noexcept -> Service& {
	if (this != &prev_v) {
		auto tmp_v{ std::move(prev_v) };
		tmp_v.Swap(*this); }
	return *this;
}

Service::~Service() {
	Stop();
}

auto Service::Swap(Service& prev_v) noexcept -> void {
	std::swap(m_Scheduler, prev_v.m_Scheduler);
	std::swap(m_State, prev_v.m_State);
}

auto Service::Pause() -> void {
	assert(m_State);
	auto& state_v{ *m_State };
	if (state_v.m_Waiting) return;
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_State = kPaused;
	state_v.m_CondVar.wait(lock_v, [this, &state_v]()->bool{
		return kPaused==state_v.m_State
		  	|| state_v.m_StopTarget.stop_requested();		
	});
}

auto Service::Stop() -> void {	
	if (!m_State) return;
	auto& state_v{ *m_State };
	state_v.m_StopSource.request_stop();		
	state_v.m_CondVar.notify_all();
	state_v.m_Thread.join();	
	m_State.reset();
} 
auto Service::Resume() -> void {
	assert(m_State);
	auto& state_v{ *m_State };
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_State = kRunning;
	state_v.m_CondVar.notify_all();	
}

auto Service::StopRequested() -> bool {
	assert(m_State);
	Yield();
	auto& state_v{ *m_State };
	std::unique_lock lock_v{ state_v.m_Mutex };
	auto result_v{ state_v.m_StopTarget.stop_requested() };
	if (result_v) state_v.m_State = kStopped;
	return result_v;
}

auto Service::Yield() -> void {
	assert(m_State);
	auto& state_v{ *m_State };
	std::unique_lock lock_v{ state_v.m_Mutex };
	state_v.m_CondVar.notify_one();
	state_v.m_Waiting = true;
	state_v.m_CondVar.wait(lock_v, [this, &state_v]()->bool{
		return kRunning==state_v.m_State 
			  || state_v.m_StopTarget.stop_requested(); 
	});
	state_v.m_Waiting = false;
}

auto Service::GetScheduler() const -> Scheduler& {
	return *m_Scheduler;
}
