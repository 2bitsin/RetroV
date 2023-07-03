#include <core/scheduler.hpp>

using core::Scheduler;

Scheduler::Scheduler(Hypervisor& hypervisor)
	: m_Hypervisor(&hypervisor)
{}

Scheduler::Scheduler(Scheduler&& prev_v) noexcept
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_Services()
	, m_FreeHandles(std::move(prev_v.m_FreeHandles))
{}

auto Scheduler::operator=(Scheduler&& prev_v) noexcept -> Scheduler& {
	if (this != &prev_v) {
		auto tmp_v{ std::move(prev_v) };
		tmp_v.Swap(*this); }
	return *this;
}

auto Scheduler::Swap(Scheduler& prev_v) noexcept -> void {	
	std::swap(m_Hypervisor, prev_v.m_Hypervisor);
	std::swap(m_Services, prev_v.m_Services);
	std::swap(m_FreeHandles, prev_v.m_FreeHandles);
}

auto Scheduler::DeviceStart(device::Interface& device_v) -> std::size_t {
	auto callback_v = [&device_v] <typename...T>(T&&...args_v) {
		device_v.Emulate(std::forward<T>(args_v)...);
	};
	using device::Interface;	
	if (m_FreeHandles.empty()) {
		auto handle_v=m_Services.size();
		m_Services.emplace_back(*this, std::move(callback_v));
		return handle_v + 1u;;
	} else {
		auto handle_v=m_FreeHandles.front();
		m_FreeHandles.pop_front();		
		m_Services[handle_v] = Service(*this, std::move(callback_v));
		return handle_v+1u;
	}
}

auto Scheduler::DeviceStop(std::size_t index_v) -> void
{
	index_v -= 1u;
	auto& service_v{ m_Services.at(index_v) };
	service_v.Stop();
}

auto Scheduler::DevicePause(std::size_t index_v) -> void
{
	index_v -= 1u;
	auto& service_v{ m_Services.at(index_v) };
	service_v.Pause();
}

auto Scheduler::DeviceResume(std::size_t index_v) -> void
{
	index_v -= 1u;
	auto& service_v{ m_Services.at(index_v) };
	service_v.Resume();
}


Scheduler::~Scheduler() {
	for (auto& service_v : m_Services) {
		service_v.Stop();
	}
}