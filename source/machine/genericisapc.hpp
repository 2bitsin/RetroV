#pragma once

#include <core/hypervisor.hpp>
#include <device/interface.hpp>

namespace machine
{
	struct GenericISAPC
	{
		GenericISAPC (std::span<char const* const> args_v);
		~GenericISAPC ();

	protected:
		static auto InitializeConfig(std::span<char const* const> args_v) -> core::Config;

	private:
		core::Hypervisor m_Hypervisor;
		std::vector<device::Interface> m_Devices;
	};
}