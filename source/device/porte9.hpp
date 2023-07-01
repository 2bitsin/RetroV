#pragma once

#include <core/hypervisor_fwd.hpp>>

#include <cstdint>
#include <cstddef>

namespace core
{
	struct PortE9
	{
		PortE9 (Hypervisor&);
		~PortE9 ();
		
		PortE9 (const PortE9&) = delete;
		PortE9 (PortE9&&) = delete;
		auto operator = (const PortE9&) -> PortE9& = delete;
		auto operator = (PortE9&&)-> PortE9& = delete;

	private:
		Hypervisor& m_Hypervisor;
	};
}