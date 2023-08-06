#pragma once

#include <thread>
#include <mutex>


namespace core
{

	struct Processor;

	struct CpuThread
	{
		CpuThread();
		~CpuThread();

    CpuThread(CpuThread const&) = delete;
		auto operator=(CpuThread const&) -> CpuThread& = delete;

		CpuThread(CpuThread&&) = delete;
		auto operator=(CpuThread&&) -> CpuThread& = delete;

		auto Start(Processor& machine_v) -> void;
		auto Stop() -> void;

		auto Suspend() -> void;
		auto Resume() -> void;

	protected:
		static auto EntryPoint (std::stop_token stopper_v, core::Processor& this_v) -> void;

	private:
		std::jthread m_Thread;
	};
}