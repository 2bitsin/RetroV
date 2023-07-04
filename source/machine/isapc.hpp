#pragma once

#include <core/hypervisor.hpp>
#include <device/interface.hpp>

namespace machine
{
	struct ISAPC
	{
		ISAPC (std::span<char const* const> args_v);
	  ~ISAPC ();

	  auto PowerOn() -> void;
		auto Shutdown() -> void;

	protected:
		static auto InitializeConfig(std::span<char const* const> args_v) -> core::Config;

	private:
		core::Hypervisor m_Hypervisor;
		std::vector<device::InterfacePtr> m_Devices;
	};
}