#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <shared_mutex>
#include <stop_token>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <thread>

namespace core
{
	struct VirtualMachine;

	struct ProcessorThread
	{
		ProcessorThread () = default;

		ProcessorThread (ProcessorThread const&) = delete;
		auto operator = (ProcessorThread const&) -> ProcessorThread& = delete;

		ProcessorThread (ProcessorThread&&) noexcept = delete;
		auto operator = (ProcessorThread&&) noexcept -> ProcessorThread& = delete;

		~ProcessorThread () noexcept;

		auto Start(VirtualMachine& machine_v, win32::WHvProcessor processor_v) -> void;
		auto Stop() -> void;


	protected:
		auto RunProcessor(std::stop_token token_v, VirtualMachine& machine_v, win32::WHvProcessor processor_v) -> void;

	private:
		mutable std::shared_mutex m_Mutex;
		std::jthread m_Thread;
	};
}
