#include <core/legacypic.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <utils/logger.hpp>

using core::LegacyPic;

LegacyPic::LegacyPic(Machine& machine_v, std::uint32_t bsp_index_v)
	: m_Machine{ machine_v }	
	, m_Processor{ machine_v.GetProcessor(bsp_index_v) }
{}

auto LegacyPic::Initialize() -> std::int32_t
{
	return S_OK;
}

auto LegacyPic::IoPortAccess(Processor const& vcpu_v, MasterOrSlave select_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
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

auto LegacyPic::InterruptWindow() -> std::int32_t
{
  return S_OK;
}

auto LegacyPic::SetIRQ(std::uint8_t state_v) -> std::int32_t
{
	using utils::logger;

	if (m_last_irr == state_v)
		return S_OK;

	m_last_irr = std::exchange(m_irr, state_v);
	auto const falling_edge_v = m_last_irr & ~m_irr;
	auto const rising_edge_v = ~m_last_irr & m_irr;


	return S_OK;
}



