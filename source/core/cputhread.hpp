#pragma once

#include <thread>
#include <mutex>

#include <utils/barrier.hpp>

namespace core
{

	struct Machine;

	struct CpuThread
	{
		CpuThread();
		~CpuThread();

    CpuThread(CpuThread const&) = delete;
		auto operator=(CpuThread const&) -> CpuThread& = delete;

		CpuThread(CpuThread&&) = delete;
		auto operator=(CpuThread&&) -> CpuThread& = delete;

		auto Start(Machine& machine_v) -> void;
		auto Stop() -> void;

		auto Suspend() -> void;
		auto Resume() -> void;

	protected:
		static auto EntryPoint (std::stop_token stopper_v, Machine& machine_v, CpuThread& this_v) -> void;

	private:
		utils::barrier m_Barrier;
		std::jthread m_Thread;
	};
}