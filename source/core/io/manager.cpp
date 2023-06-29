#include <core/io/manager.hpp>
#include <core/hypervisor.hpp>
#include <utils/bitmanip.hpp>
#include "manager.hpp"

using core::io::Manager;

Manager::Manager(core::Hypervisor& hypervisor_v)
	: m_Hypervisor(hypervisor_v)
{
	m_IoFetchCallback.reserve(0x10000u);
	m_IoWriteCallback.reserve(0x10000u);
}

auto Manager::RegisterWriteCallback(std::uint16_t port_v, write_callback callback_v) -> void
{
	if (port_v >= m_IoWriteCallback.size()) {
		m_IoWriteCallback.resize(port_v + 1);
	}
	if (m_IoWriteCallback[port_v]) {
		throw std::logic_error("fetch callback already registered");
	}
	m_IoWriteCallback[port_v] = std::move(callback_v);
}

auto Manager::RegisterFetchCallback(std::uint16_t port_v, fetch_callback callback_v) -> void
{
	if (port_v >= m_IoFetchCallback.size()) {
		m_IoFetchCallback.resize(port_v + 1);		
	}
	if (m_IoFetchCallback[port_v]) {
		throw std::logic_error("fetch callback already registered");
	}
	m_IoFetchCallback[port_v] = std::move(callback_v);
}

auto Manager::UnregisterWriteCallback(std::uint16_t port_v) -> void
{
	m_IoWriteCallback[port_v] = std::function<write_callback>{};
}

auto Manager::UnregisterFetchCallback(std::uint16_t port_v) -> void
{
	m_IoFetchCallback[port_v] = std::function<fetch_callback>{};
}

auto Manager::DispatchWrite(std::uint32_t index_v, std::uint16_t port_v, std::uint32_t data_v, std::uint8_t size_v) -> bool
{
	if (port_v >= m_IoWriteCallback.size()) {
		return false;
	}

	if (auto& callback_v = m_IoWriteCallback[port_v];	callback_v) {
		return callback_v(m_Hypervisor, index_v, port_v, data_v, size_v);		
	}

	return false;
}
auto Manager::DispatchFetch(std::uint32_t index_v, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v) -> bool
{
	if (port_v >= m_IoFetchCallback.size()) {
		return false;
	}

	if (auto& callback_v = m_IoFetchCallback[port_v]; callback_v) {
		return callback_v(m_Hypervisor, index_v, port_v, data_v, size_v);
	}

	return false;
}

auto Manager::DispatchExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	auto &access_v = exit_v.IoPortAccess;
	auto port_v = (std::uint16_t)access_v.PortNumber;
	auto data_v = (std::uint32_t)access_v.Rax;
	auto size_v = access_v.AccessInfo.AccessSize;
	
	if (!access_v.AccessInfo.IsWrite) {
		DispatchFetch(index_v, port_v, data_v, size_v);
		auto& processor_v = m_Hypervisor.GetProcessor(index_v);
		processor_v.SetRegister(WHvX64RegisterRax, utils::crossover_bits(
			(std::uint64_t)data_v, access_v.Rax, size_v * 8u));
	} else {
		DispatchWrite(index_v, port_v, data_v, size_v);
	}

	return true;
}
