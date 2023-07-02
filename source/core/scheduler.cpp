#include "scheduler.hpp"
#include "scheduler.hpp"
#include "scheduler.hpp"
#include "scheduler.hpp"
#include "scheduler.hpp"
#include "scheduler.hpp"
#include "scheduler.hpp"
#include <core/scheduler.hpp>

using core::Scheduler;

Scheduler::Scheduler(Hypervisor& hypervisor)
	: m_Hypervisor(&hypervisor)
	, m_Devices()
{}

Scheduler::Scheduler(Scheduler&& prev_v) noexcept
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_Devices(std::move(prev_v.m_Devices))
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
	std::swap(m_Devices, prev_v.m_Devices);
	std::swap(m_FreeHandles, prev_v.m_FreeHandles);
}

auto Scheduler::DeviceStart(device::Interface& devifc_v) -> std::size_t
{
	std::size_t handle_v{ 0 };
	if (!m_FreeHandles.empty()) {
		handle_v = m_FreeHandles.front();
		m_FreeHandles.pop_front();
	} else {
		handle_v = m_Devices.size();
		m_Devices.emplace_back(DeviceContext{}); }
	auto& context_v{ m_Devices[handle_v] };
	context_v.m_Device = &devifc_v;
	DeviceResume(context_v);
	return handle_v;
}

auto Scheduler::DevicePause(std::size_t index_v) -> void {
	if (index_v >= m_Devices.size()) {
		throw std::out_of_range("Invalid device handle"); }
	auto& context_v{ m_Devices[index_v] };
	return DevicePause(context_v);
}

auto Scheduler::DeviceResume(std::size_t index_v) -> void
{
	if (index_v >= m_Devices.size()) {
		throw std::out_of_range("Invalid device handle"); }
	auto& context_v{ m_Devices[index_v] };
	return DeviceResume(context_v);
}

auto Scheduler::DeviceStop(DeviceContext& context_v) -> void {
	DevicePause(context_v);
	context_v.m_Device = nullptr;
}

auto Scheduler::DevicePause(DeviceContext& context_v) -> void
{
	if (context_v.m_Device == nullptr) {
		throw std::runtime_error("Invalid device handle"); }
	context_v.m_StopSource.request_stop();
	context_v.m_Thread.join();
}

auto Scheduler::DeviceResume(DeviceContext& context_v) -> void
{
	if (context_v.m_Device == nullptr) {
		throw std::runtime_error("Invalid device handle"); }
	context_v.m_StopSource = std::stop_source{};
	context_v.m_Thread = std::jthread{ [&context_v]() {
		auto token_v{ context_v.m_StopSource.get_token() };
		auto& device_v = *context_v.m_Device;
		device_v.Emulate(token_v);
	}};
}

auto Scheduler::DeviceStop(std::size_t index_v) -> void {
	auto& context_v{ m_Devices[index_v] };
	DeviceStop(context_v);	
	m_FreeHandles.push_back(index_v);
}

Scheduler::~Scheduler() 
{
	for (auto& context_v : m_Devices) {
		if (context_v.m_Device != nullptr) {
			DeviceStop(context_v); }}
}