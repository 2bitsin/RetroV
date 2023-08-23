#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

#include <utils/span.hpp>

namespace core
{ 
	struct Machine;
	struct Processor;
	struct LocalApic;

	struct LegacyPic
	{
		enum MasterOrSlave { Master = 0u, Slave = 1u } ;

		LegacyPic (Machine& machine_v, std::uint32_t bsp_index_v);
		
		auto Initialize() -> std::int32_t;	
		auto IoPortAccess(Processor const& vcpu_v, MasterOrSlave select_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto InterruptWindow() -> std::int32_t;

		auto SetIRQ(std::uint8_t state_v) -> std::int32_t;

	private:
		std::mutex m_lock;
	
		std::uint16_t m_last_irr { 0u };

		std::uint16_t m_irr { 0u };
		std::uint16_t m_isr { 0u };

		Machine& m_Machine;
		Processor& m_Processor;
	};
}