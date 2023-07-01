#pragma once

#include <core/hypervisor.hpp>
#include <device/interface.hpp>

namespace machine
{
	struct GenericISAPC
	{

	private:
		core::Hypervisor m_Hypervisor;
		std::vector<device::Interface> m_Devices;
	};
}