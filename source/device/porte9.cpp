#include <device/porte9.hpp>
#include <core/hypervisor.hpp>

#include <iostream>

using device::PortE9;


PortE9::PortE9(core::Hypervisor& hypervisor_v)
	: m_Hypervisor { &hypervisor_v }	
{
	m_Hypervisor->GetIoManager().RegisterWriteCallback(0xe9, 
		[](auto& hypervisor_v, auto& processor_v, auto port_v, auto value_v, auto size_v) -> bool {
			if (port_v != 0xe9) return false;
			std::cerr << static_cast<char>(value_v & 0xffu);
			return true;
		});
}

PortE9::~PortE9()
{
	m_Hypervisor->GetIoManager().UnregisterWriteCallback(0xe9);
}