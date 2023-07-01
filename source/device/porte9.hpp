#pragma once

#include <device/interface.hpp>

#include <cstdint>
#include <cstddef>

namespace device
{
	struct PortE9
	{
		PortE9 (core::Hypervisor&);
		~PortE9 ();
		
		PortE9 (const PortE9&) = delete;
		PortE9 (PortE9&&) = delete;
		auto operator = (const PortE9&) -> PortE9& = delete;
		auto operator = (PortE9&&)-> PortE9& = delete;

	private:
		core::Hypervisor* m_Hypervisor;
	};
}