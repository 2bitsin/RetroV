#pragma once

#include <win32/windows.hpp>
#include <win32/filetime_clock.hpp>

#include <type_traits>
#include <functional>
#include <chrono>
#include <memory>

namespace win32
{
	struct WorkQueue;

	/////////////////
	// 
	// WorkInstance
	// 
	/////////////////

	struct WorkInstance
	{
		WorkInstance(PTP_CALLBACK_INSTANCE instance_v) noexcept;

		auto MayRunLong() const -> bool;

	private:
		PTP_CALLBACK_INSTANCE m_Handle{ nullptr };
	};

	/////////////////
	// 
	// WorkItem
	// 
	/////////////////

	struct WorkItem: std::enable_shared_from_this<WorkItem>
	{
		template <typename Callback>
		requires (std::is_invocable_v<Callback, WorkInstance, WorkItem&>)
		WorkItem(Callback&& callback_v);

		~WorkItem();

		WorkItem(WorkItem const&) = delete;
		auto operator=(WorkItem const&) -> WorkQueue & = delete;

		WorkItem (WorkItem&&) noexcept;
		auto operator = (WorkItem&&) noexcept -> WorkItem&;

		auto swap(WorkItem&) noexcept -> void;

		auto Handle() const noexcept -> PTP_WORK;

		auto Wait() const -> void;
		auto Cancel () const -> void;

		auto SubmitTo(WorkQueue& queue_v) -> void;

		static void NTAPI EntryPoint(PTP_CALLBACK_INSTANCE instance_v, void* context_v, PTP_WORK work_v);

	private:
		std::function<void(WorkInstance, WorkItem&)> m_Cbkfun;
		PTP_WORK m_Handle{ nullptr };
	};


	template<typename Callback>
		requires (std::is_invocable_v<Callback, WorkInstance, WorkItem&>)
	inline WorkItem::WorkItem(Callback&& callback_v)
		: m_Cbkfun(std::forward<Callback>(callback_v))
		, m_Handle(nullptr)
	{}

	/////////////////
	// 
	// WorkTimer
	// 
	/////////////////

	struct WorkTimer
	{
		using duration_100ns = std::chrono::milliseconds;
		using time_point_type = win32::filetime_clock::time_point;

		template <typename Callback>
		requires (std::is_invocable_v<Callback, WorkInstance, WorkTimer&>)
		WorkTimer(Callback&& callback_v);

		~WorkTimer();

		WorkTimer(WorkTimer const&) = delete;
		auto operator=(WorkTimer const&) -> WorkTimer & = delete;

		WorkTimer(WorkTimer &&) noexcept;
		auto operator=(WorkTimer &&) noexcept -> WorkTimer &;

		auto swap(WorkTimer&) -> void;

		auto Handle() const noexcept -> PTP_TIMER;		
		auto Cancel() -> void;
		
		auto SubmitTo(WorkQueue& queue_v, time_point_type expire_v,
			duration_100ns period_v = duration_100ns::zero()) -> void;
		
		auto SubmitTo(WorkQueue& queue_v, duration_100ns expire_v,
			duration_100ns period_v = duration_100ns::zero()) -> void;

	protected:
		
		auto SubmitTo(WorkQueue& queue_v, FILETIME expire_v, std::uint32_t period_millisec_v) -> void;

		static auto NTAPI EntryPoint(PTP_CALLBACK_INSTANCE instance_v, void* context_v, PTP_TIMER timer_v) -> void;
	private:
		std::function<void(WorkInstance, WorkTimer&)> m_Cbkfun;
		PTP_TIMER m_Handle{ nullptr };
	};

	template<typename Callback>
	requires (std::is_invocable_v<Callback, WorkInstance, WorkTimer&>)
	inline WorkTimer::WorkTimer(Callback&& callback_v)
		: m_Cbkfun(std::forward<Callback>(callback_v))
		, m_Handle(nullptr)
	{}

	/////////////////
	// 
	// WorkQueue
	// 
	/////////////////

	struct WorkQueue
	{
		WorkQueue();
		~WorkQueue();

		WorkQueue(WorkQueue const&) = delete;
		auto operator=(WorkQueue const&) -> WorkQueue & = delete;

		WorkQueue(WorkQueue &&) noexcept;
		auto operator=(WorkQueue &&) noexcept -> WorkQueue &;

		auto swap(WorkQueue&) -> void;

		template <typename Callback>
		requires (std::is_invocable_v<Callback, WorkInstance>)
		auto Submit(Callback&&) -> bool;

		auto Submit(WorkItem& work_v) -> void;

		template <typename... Args>
		auto Submit(WorkTimer& work_v, Args&&... args_v) -> void;

		auto Handle() const noexcept -> PTP_POOL;
		auto Cbkenv() noexcept -> TP_CALLBACK_ENVIRON&;
		auto Cbkenv() const noexcept -> TP_CALLBACK_ENVIRON const&;

	private:
		PTP_POOL m_Handle{ nullptr };
		TP_CALLBACK_ENVIRON m_Cbkenv;
	};

	template<typename Callback>
	requires (std::is_invocable_v<Callback, WorkInstance>)
	inline auto WorkQueue::Submit(Callback&& callback_v) -> bool
	{
		using callback_type = std::remove_cvref_t<Callback>;
		return !!::TrySubmitThreadpoolCallback(
			[] (PTP_CALLBACK_INSTANCE instance_v, void* context_v) noexcept {
				auto const callback_v = static_cast<Callback*>(context_v);
				(*callback_v)(WorkInstance{ instance_v }); 
				delete callback_v; }, 
			new callback_type{ std::forward<Callback>(callback_v) },
			&m_Cbkenv);
	}

	template<typename ...Args>
	inline auto WorkQueue::Submit(WorkTimer& timer_v, Args&&... args_v) -> void
	{
		return timer_v.SubmitTo(*this, std::forward<Args>(args_v)...);
	}
}