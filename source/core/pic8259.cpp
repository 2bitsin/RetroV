#include <core/pic8259.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>
#include <core/localapic.hpp>

using core::Pic8259;

Pic8259::Pic8259(Machine& machine_v, std::uint32_t bsp_index_v)
	: m_Machine{ machine_v }	
	, m_Processor{ machine_v.GetProcessor(bsp_index_v) }
	, m_LocalApic{ machine_v.GetLocalApic(bsp_index_v) }
{}

auto Pic8259::Initialize() -> std::int32_t
{
	auto result_v = m_LocalApic.XApicWrite(m_LocalApic.SIVR, m_LocalApic.SIVR_APIC_ENABLED + 0xFu);
	if (FAILED(result_v)) {
		return result_v;
	}
	
}

auto Pic8259::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	switch (port_v)
	{
	case 0x20:
	case 0xa0:
		return S_OK;

	case 0x21:
		if (data_v.size() == 1u && data_v.as<std::uint8_t>() == 0x20u) {
			m_LocalApic.SignalEOI();
		}
		[[falltrough]]
	case 0xa1:
		return S_OK;

	default:
		__debugbreak();
		return S_OK;
	}
}

