#pragma once

#include <core/iodevice.hpp>
#include <core/machine.hpp>

namespace core
{
	template <typename _ActualDevice>
	struct IODeviceImpl final : public IODevice
	{
		IODeviceImpl(Machine& machine_v, _ActualDevice actual_device_v, std::uint16_t port_base_v, std::uint16_t port_end_v)
			requires (std::is_reference_v<_ActualDevice>)
			: m_Machine(machine_v)
			, m_ActualDevice(actual_device_v)
			, m_PortBase(port_base_v)
			, m_PortEnd(port_end_v)
		{
			m_Machine.MapIoRange(*this, m_PortBase, m_PortEnd - m_PortBase);
		}
	
		template <typename T>
		IODeviceImpl(Machine& machine_v, T&& actual_device_v, std::uint16_t port_base_v, std::uint16_t port_end_v)
			requires (!std::is_reference_v<_ActualDevice>)
			: m_Machine(machine_v)
			, m_ActualDevice(std::forward<T>(actual_device_v))
			, m_PortBase(port_base_v)
			, m_PortEnd(port_end_v)
		{
			m_Machine.MapIoRange(*this, m_PortBase, m_PortEnd - m_PortBase);
		}
	
		~IODeviceImpl() {
			m_Machine.UnmapIoRange(m_PortBase, m_PortEnd - m_PortBase);
		}
	
		auto PortWrite(std::uint16_t port_v, std::uint64_t  value_v, std::size_t size_v) -> bool override final {
			return m_ActualDevice.PortWrite(port_v, value_v, size_v);
		}
		auto PortFetch(std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool override final {
			return m_ActualDevice.PortFetch(port_v, value_v, size_v);
		}
	private:
		Machine& m_Machine;
		_ActualDevice m_ActualDevice;
		std::uint16_t m_PortBase;
		std::uint16_t m_PortEnd;
	};

}