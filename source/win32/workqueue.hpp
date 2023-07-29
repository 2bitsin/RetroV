#pragma once

#include <win32/windows.hpp>
#include <type_traits>

namespace win32
{
	struct WorkQueue;

	struct Work
	{
		template <typename Callback>
		Work(WorkQueue& queue_v, Callback&& callback_v);
		~Work();

		Work(Work const&) = delete;
		auto operator=(Work const&) -> WorkQueue & = delete;

		Work(Work&&);
		auto operator = (Work&) -> Work&;

	private:
		PTP_WORK m_Handle{ nullptr };
	};

	struct WorkQueue
	{
		WorkQueue();
		~WorkQueue();

		WorkQueue(WorkQueue const&) = delete;
		auto operator=(WorkQueue const&) -> WorkQueue & = delete;

		WorkQueue(WorkQueue &&);
		auto operator=(WorkQueue &&) -> WorkQueue &;

		auto Handle () const -> TP_POOL*;
		auto swap (WorkQueue&) -> void;

		auto Cbkenv() -> TP_CALLBACK_ENVIRON&;

	private:
		TP_POOL* m_Handle{ nullptr };
		TP_CALLBACK_ENVIRON m_Cbkenv;
	};

	template<typename Callback>
	inline Work::Work(WorkQueue& queue_v, Callback&& callback_v)
	{
		using callback_type = std::remove_cvref_t<Callback>;
		::CreateThreadpoolWork(
			[] (PTP_CALLBACK_INSTANCE instance_v, PVOID context_v, PTP_WORK work_v) {
				
			}, 
			this, &queue_v.Cbkenv()
		);
	}

}