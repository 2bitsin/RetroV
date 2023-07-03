#pragma once

#include <device/interface.hpp>
#include <core/hypervisor_fwd.hpp>
#include <core/service.hpp>

#include <condition_variable>
#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <thread>
#include <deque>
#include <mutex>

namespace core
{
	struct Scheduler 
	{
		Scheduler(Hypervisor&);

		Scheduler(Scheduler const&) = delete;
		auto operator=(Scheduler const&) -> Scheduler& = delete;

		Scheduler(Scheduler&&) noexcept;
		auto operator=(Scheduler&&) noexcept -> Scheduler&;

		auto Swap(Scheduler&) noexcept -> void;

		auto DeviceStart(device::Interface&) -> std::size_t;
		auto DeviceStop(std::size_t) -> void;
		auto DevicePause(std::size_t) -> void;
		auto DeviceResume(std::size_t) -> void;

		~Scheduler();

	private:
		Hypervisor* m_Hypervisor { nullptr };
		std::vector<Service> m_Services;
		std::deque<std::size_t> m_FreeHandles;

	};
}