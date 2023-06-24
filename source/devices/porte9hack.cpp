#include <devices/porte9hack.hpp>
#include <core/machine.hpp>

#include <iostream>

using core::PortE9HackDevice;

auto PortE9HackDevice::PortWrite(std::uint16_t port_v, std::uint64_t value_v, std::size_t size_v) -> bool
{
	if (port_v != 0xe9) return false;
	std::cout << static_cast<char>(value_v & 0xffu);
	return true;
}

auto PortE9HackDevice::PortFetch(std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool
{
	return false;
}

PortE9HackDevice::PortE9HackDevice(Machine& machine_v)
	: m_Machine { machine_v }
	, m_IoRange { m_Machine, *this, 0xe9u, 0xeau }
{}

PortE9HackDevice::~PortE9HackDevice()
{}