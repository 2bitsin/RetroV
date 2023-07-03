#pragma once
#include <condition_variable>
#include <functional>
#include <stop_token>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>

namespace core {

	struct Scheduler;

	enum ServiceState {
		kUndefined,
		kNotStarted,
		kRunning,
		kPaused,
		kStopped
	};


	struct Service
	{
		using task_type = void(Scheduler&,Service&);

		Service() noexcept;
		Service(Scheduler&, std::function<task_type> callback_v);

		Service(Service&&) noexcept;
		auto operator = (Service&&) noexcept -> Service&;

	  ~Service();

		Service(Service const&) = delete;
		auto operator = (Service const&) = delete;

		auto Swap (Service&) noexcept -> void;

		auto Stop () -> void;		
		auto Resume () -> void;
		auto Pause () -> void;

		auto GetScheduler() const -> Scheduler& ;
		auto Yield() -> void;
		auto StopRequested() -> bool;

	protected:
		struct state_type
		{
			std::mutex m_Mutex;
			std::atomic<ServiceState> m_State;
			std::atomic<bool> m_Waiting;
			std::condition_variable m_CondVar;
			std::stop_source m_StopSource;
			std::stop_token m_StopTarget;
			std::jthread m_Thread;
		};

	private:
		Scheduler* m_Scheduler{ nullptr };
		std::unique_ptr<state_type> m_State;
	};

}