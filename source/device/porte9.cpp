#include <device/porte9.hpp>
#include <core/hypervisor.hpp>

#include <iostream>

using device::PortE9;


PortE9::PortE9(core::Hypervisor& hypervisor_v, Config const&)
	: m_Hypervisor { &hypervisor_v }	
{
	m_Hypervisor->GetEventBroker().ConnectIoWrite(0xE9u, 1u, this);
}

PortE9::~PortE9()
{}

auto PortE9::IoWrite(core::Processor& vcpu_v, std::uint16_t port_v, std::uint64_t data_v, std::uint8_t size_v) -> bool
{
	if (port_v != 0xE9u) return false;
	std::cout << static_cast<char>(data_v);
	return true;
}

auto PortE9::Emulate(core::Scheduler&, core::Service&) -> void
{}

auto PortE9::GetCategory() const noexcept 
	-> device::DeviceCatory
{ return DeviceCatory::kDebug; }

auto PortE9::Pause() -> void
{}

auto PortE9::Resume() -> void
{}
