#pragma once

#include <cstdint>
#include <cstddef>

#include <core/iodevice.hpp>
#include <core/iodeviceimpl.hpp>

namespace core
{
	struct Machine;

	struct PortE9HackDevice
	{
		PortE9HackDevice (Machine&);
		~PortE9HackDevice ();
		
		PortE9HackDevice (const PortE9HackDevice&) = delete;
		PortE9HackDevice (PortE9HackDevice&&) = delete;
		auto operator = (const PortE9HackDevice&) -> PortE9HackDevice& = delete;
		auto operator = (PortE9HackDevice&&)-> PortE9HackDevice& = delete;

		auto PortWrite (std::uint16_t port_v, std::uint64_t  value_v, std::size_t size_v) -> bool;
		auto PortFetch (std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool;

	private:
		Machine& m_Machine;
		IODeviceImpl<PortE9HackDevice&> m_IoRange;
	};
}