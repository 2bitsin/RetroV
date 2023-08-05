#pragma once

#include <condition_variable>
#include <atomic>
#include <mutex>

namespace utils
{
	struct barrier
	{
		barrier(bool wait_v = false):
			m_wait{ wait_v } 
		{}

		auto wait() -> void 
		{
			std::unique_lock lock_v{ m_mutex };
			if (!m_wait) 
				return;
			m_cndvar.wait(lock_v, [this]() { 
				return !m_wait; 
			});					
		}

		auto lock(bool wait_v) -> void
		{
			std::unique_lock lock_v{ m_mutex };
			m_wait = wait_v;
			if (!m_wait) {
				m_cndvar.notify_all();
			}
		}

	private:
		std::condition_variable m_cndvar;
		std::mutex m_mutex;
		bool m_wait;
	};
}