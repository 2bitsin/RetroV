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
		using task_type = void(Service&);

		Service(std::function<task_type> callback_v);


	protected:
		struct state_type
		{
			std::mutex m_Mutex;
			ServiceState m_State;
			std::conditional_variable m_CondVar;
			std::stop_source m_StopSource;
			std::stop_token m_StopTarget;
			std::jthread m_Thread;
		};

	private:
		std::unique_ptr<state_type> m_State;
	};

}