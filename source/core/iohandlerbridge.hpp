#pragma once

#include <core/iohandler.hpp>
#include <core/machine.hpp>

namespace core
{
	template <typename _Handler>
	struct IOHandlerBridge final : public IOHandler
	{
		IOHandlerBridge(Machine& machine_v, std::uint16_t port_base_v, std::uint16_t port_end_v, _Handler what_v)
			requires (std::is_reference_v<_Handler>)
			: m_Machine(machine_v)
			, m_Handler(what_v)
			, m_PortBase(port_base_v)
			, m_PortEnd(port_end_v)
		{
			m_Machine.MapIoRange(*this, m_PortBase, m_PortEnd - m_PortBase);
		}
	
		template <typename... T>
		IOHandlerBridge(Machine& machine_v, std::uint16_t port_base_v, std::uint16_t port_end_v, T&&... args_v)
			requires (!std::is_reference_v<_Handler>)
			: m_Machine(machine_v)
			, m_Handler(std::forward<T>(args_v)...)
			, m_PortBase(port_base_v)
			, m_PortEnd(port_end_v)
		{
			m_Machine.MapIoRange(*this, m_PortBase, m_PortEnd - m_PortBase);
		}
	
		~IOHandlerBridge() {
			m_Machine.UnmapIoRange(m_PortBase, m_PortEnd - m_PortBase);
		}
	
		auto PortWrite(std::uint16_t port_v, std::uint64_t  value_v, std::size_t size_v) -> bool override final {
			return m_Handler.PortWrite(port_v, value_v, size_v);
		}
		auto PortFetch(std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool override final {
			return m_Handler.PortFetch(port_v, value_v, size_v);
		}
	private:
		Machine& m_Machine;
		_Handler m_Handler;
		std::uint16_t m_PortBase;
		std::uint16_t m_PortEnd;
	};

}