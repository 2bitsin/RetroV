#pragma once

#include <cstdint>
#include <cstddef>


namespace core
{
	struct Hypervisor;

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