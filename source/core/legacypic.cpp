#include <core/legacypic.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <utils/logger.hpp>

using core::LegacyPic;

LegacyPic::LegacyPic(Machine& machine_v, uint32_t bsp_index_v)
	: m_Master		{ *this, MasterOrSlave::Master }
	, m_Slave			{ *this, MasterOrSlave::Slave }
	, m_Machine		{ machine_v }	
	, m_Processor	{ machine_v.GetProcessor(bsp_index_v) }
{}

auto LegacyPic::Initialize() ->void
{
}

auto LegacyPic::Reset() ->void
{
}

auto LegacyPic::IoPortAccess(Processor const& vcpu_v, MasterOrSlave select_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t
{
	switch (port_v)
	{
	case 0x00U:
	case 0x01U:
		return S_OK;

	default:
		return S_OK;
	}
}

auto LegacyPic::InterruptWindow() -> void
{
  
}

auto LegacyPic::SetIRQ(uint8_t state_v) -> void
{
	using utils::logger;

	if (m_last_irr == state_v)
		return;

	m_last_irr = std::exchange(m_irr, state_v);
	auto const falling_edge_v = m_last_irr & ~m_irr;
	auto const rising_edge_v = ~m_last_irr & m_irr;
	
}



