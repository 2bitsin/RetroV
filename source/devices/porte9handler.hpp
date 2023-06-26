#pragma once

#include <cstdint>
#include <cstddef>

#include <core/iohandler.hpp>
#include <core/iohandlerbridge.hpp>

namespace core
{
	struct Machine;

	struct PortE9Handler
	{
		PortE9Handler (Machine&);
		~PortE9Handler ();
		
		PortE9Handler (const PortE9Handler&) = delete;
		PortE9Handler (PortE9Handler&&) = delete;
		auto operator = (const PortE9Handler&) -> PortE9Handler& = delete;
		auto operator = (PortE9Handler&&)-> PortE9Handler& = delete;

		auto PortWrite (std::uint16_t port_v, std::uint64_t  value_v, std::size_t size_v) -> bool;
		auto PortFetch (std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool;

	private:
		Machine& m_Machine;
		IOHandlerBridge<PortE9Handler&> m_IoRange;
	};
}