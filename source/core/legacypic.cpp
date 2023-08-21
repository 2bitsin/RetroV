#include <core/legacypic.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

using core::LegacyPic;

LegacyPic::LegacyPic(Machine& machine_v, std::uint32_t bsp_index_v)
	: m_Machine{ machine_v }	
	, m_Processor{ machine_v.GetProcessor(bsp_index_v) }
{}

auto LegacyPic::Initialize() -> std::int32_t
{
	return S_OK;
}

auto LegacyPic::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	switch (port_v)
	{
	case 0x20:
	case 0xa0:
		return S_OK;

	case 0x21:
	case 0xa1:
		return S_OK;

	default:
		return S_OK;
	}
}

auto LegacyPic::InterruptWindow() -> std::int32_t
{
  return S_OK;
}

auto LegacyPic::AssertLines(std::uint16_t mask_v) -> std::int32_t {
	auto [status_v, reftsc_v] = m_Processor.ReferenceTsc();
	if (FAILED(status_v))
		return status_v;
	std::lock_guard<std::mutex> lock{ m_lock };
	auto edge_v = m_irr;
	m_irr |= mask_v;
	if (edge_v ^= m_irr) {
		// rising edge
	}
	return S_OK;
}

auto LegacyPic::DeassertLines(std::uint16_t mask_v) -> std::int32_t {
	auto [status_v, reftsc_v] = m_Processor.ReferenceTsc();
	if (FAILED(status_v))
		return status_v;
	std::lock_guard<std::mutex> lock{ m_lock };
	auto edge_v = m_irr;
	m_irr &= ~mask_v;
	if (edge_v ^= m_irr) {
		// falling edge
	}
	return S_OK;
}

