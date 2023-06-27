#pragma once

#include <core/vchandler.hpp>
#include <core/machine.hpp>

namespace core 
{
	
	template <typename _Handler>
	struct VCHandlerBridge final: public VCHandler
	{
		template <typename... T>
		VCHandlerBridge(Machine& machine, std::uint16_t base, std::uint16_t end, T&&... args_v)
			requires (!std::is_reference_v<_Handler>)
			: m_Machine(machine)
			, m_Base(base)
			, m_End(end)
			, m_Handler(std::forward<T>(args_v)...)
		{
			m_Machine.MapVCHandler(m_Base, m_End, this);
		}

		VCHandlerBridge(Machine& machine, std::uint16_t base, std::uint16_t end, _Handler handler_v)
			requires (std::is_reference_v<_Handler>)
			: m_Machine(machine)
			, m_Base(base)
			, m_End(end)
			, m_Handler(handler_v)
		{
			m_Machine.MapVcRange(*this, m_Base, m_End);
		}

		~VCHandlerBridge() {
			m_Machine.UnmapVcRange(*this, m_Base, m_End);
		}

		auto VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v, std::uint16_t callno_v) -> bool override final {
			return m_Handler.VMCall(machine_v, cpuindex_v, registers_v, callno_v);
		}

		auto VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v) -> bool override final {
			return m_Handler.VMCall(machine_v, cpuindex_v, registers_v);
		}
		
	private:
		Machine& m_Machine;
		std::uint16_t m_Base;
		std::uint16_t m_End;
		_Handler m_Handler;
	};

}