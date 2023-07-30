#pragma once

#include <win32/windows.hpp>

#include <type_traits>
#include <functional>
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

		auto InitiateLongRunningTask() const -> bool;

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
		WorkItem(WorkQueue& queue_v, Callback&& callback_v);
		~WorkItem();

		WorkItem(WorkItem const&) = delete;
		auto operator=(WorkItem const&) -> WorkQueue & = delete;

		WorkItem (WorkItem&&) noexcept;
		auto operator = (WorkItem&&) noexcept -> WorkItem&;

		auto swap(WorkItem&) noexcept -> void;

		auto Handle() const noexcept -> PTP_WORK;

		auto Join(bool cancel_v) const -> void;

	protected:

		static void NTAPI EntryPoint(PTP_CALLBACK_INSTANCE instance_v, void* context_v, PTP_WORK work_v);

	private:
		std::function<void(WorkInstance, WorkItem&)> m_Cbkfun;
		PTP_WORK m_Handle{ nullptr };
	};

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

		template<typename Callback>
		auto Submit(Callback&& callback_v) -> std::shared_ptr<WorkItem> {
			auto work_ptr = std::make_shared<WorkItem>(*this, std::forward<Callback>(callback_v));
			::SubmitThreadpoolWork(work_ptr->Handle());
			return work_ptr;
		}

		auto Handle() const noexcept -> PTP_POOL;
		auto Cbkenv() noexcept -> TP_CALLBACK_ENVIRON&;
		auto Cbkenv() const noexcept -> TP_CALLBACK_ENVIRON const&;

	private:
		PTP_POOL m_Handle{ nullptr };
		TP_CALLBACK_ENVIRON m_Cbkenv;
	};


	/////////////////
	// 
	// WorkItem
	// 
	/////////////////


	template<typename Callback>
	inline WorkItem::WorkItem(WorkQueue& queue_v, Callback&& callback_v)
		: m_Cbkfun(std::forward<Callback>(callback_v))
		, m_Handle(::CreateThreadpoolWork(
				&WorkItem::EntryPoint, 
				new std::shared_ptr<WorkItem>(shared_from_this()),
				&queue_v.Cbkenv()
			))
	{
		if (nullptr== m_Handle) {
			throw error(error::last_error());
		}
	}
}