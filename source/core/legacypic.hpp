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
		enum class MasterOrSlave { Master = 0u, Slave = 1u } ;

		struct Proxy {

			friend LegacyPic;

			auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t {
				return m_ActualPic.IoPortAccess(vcpu_v, m_Which, is_write_v, port_v, data_v);
			}
		
		protected:
			Proxy (LegacyPic& actual_pic_v, MasterOrSlave which_v)
				: m_ActualPic { actual_pic_v }, m_Which { which_v }
			{}

		private:
			LegacyPic& m_ActualPic;
			MasterOrSlave m_Which;
		};

		friend Proxy;

		LegacyPic (Machine& machine_v, uint32_t bsp_index_v);
		
		auto Initialize() -> void;	
		auto Reset() -> void;
		auto InterruptWindow() -> void;
		auto SetIRQ(uint8_t state_v) -> void;

		auto Master() -> Proxy& { return m_Master; }
		auto Slave() -> Proxy& { return m_Slave; }


	protected:
		auto IoPortAccess(Processor const& vcpu_v, MasterOrSlave select_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t;

	private:
		Proxy m_Master;
		Proxy m_Slave;

		std::mutex m_lock;	
		uint16_t m_last_irr { 0u };
		uint16_t m_irr { 0u };
		uint16_t m_isr { 0u };

		Machine& m_Machine;
		Processor& m_Processor;
	};
}