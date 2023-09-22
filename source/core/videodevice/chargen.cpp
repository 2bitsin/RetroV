#include <core/videodevice/chargen.hpp>
#include <core/videodevice.hpp>

using core::videodevice::CharGen;

CharGen::CharGen(VideoDevice& device_v)
: m_Device(device_v)
{
	Reset();
}

auto CharGen::LoadRow(uint32_t address_v, uint8_t lastoff_v) -> std::int32_t
{
	using namespace size_literals;
	using std::tie;
	static constexpr auto cell_size = sizeof(m_RowCache[0]);
	auto last_addr_v = 1ull*(address_v + lastoff_v*cell_size);
	auto base_addr_v = 1ull*(address_v);
	auto last_page_v = utils::round_down(last_addr_v, 4_KiB);
	auto base_page_v = utils::round_down(base_addr_v, 4_KiB);	
	if (base_page_v == last_page_v) {
		auto const [status_v, size_v, data_s] = m_Device
			.GetMemoryRegion({ base_page_v, 4_KiB });
		if (status_v != 0) return status_v;		
		std::memcpy(&m_RowCache[0], &data_s[base_addr_v&0xFFF], 
			(lastoff_v+1u)*cell_size);
	} else {
		auto const split_point_v = 4_KiB - (base_addr_v & 0xFFFull);
		auto [status_v, size_v, data_s] = m_Device
			.GetMemoryRegion({ base_page_v, 4_KiB });
		if (status_v != 0) return status_v;
		std::memcpy(&m_RowCache[0], &data_s[base_addr_v & 0xFFF], 
			split_point_v);
		tie(status_v, size_v, data_s) = m_Device
			.GetMemoryRegion({ last_page_v, 4_KiB });
		if (status_v != 0) return status_v;
		std::memcpy(&m_RowCache[split_point_v], &data_s[0], 
			(lastoff_v+1u)*cell_size - split_point_v);
	}
	return ERROR_SUCCESS;
}

auto CharGen::Reset() -> void
{
	m_CharRows   = 15u; // 16
	m_CharCols   = 0u;  // 9
	m_ColorMode  = 1u;  // color = true
	m_BlinkPhase = 0u;  // BlinkPhase = 0
	m_TableAddr  = 32u; // TableAddr = 128k / 4k = 32
	m_DivideBy2  = 0u;  // DivideBy2 = 0

	m_CharY      = 0u;  // CharY = 0
	m_CharX      = 0u;  // CharX = 0
	m_ColIdx     = 0u;  // ColIdx = 0
}

auto CharGen::NextLine() -> std::int32_t
{
	m_ColIdx     = 0u;  // Begin at first column
	m_CharX      = 0u;	// Begin at first dot
	m_CharY      = m_CharY != m_CharRows
		           ? m_CharY + 1u : 0u;

	return ERROR_SUCCESS;
}

auto CharGen::NextFrame() -> std::int32_t
{
	m_BlinkPhase = 
	return ERROR_SUCCESS;
}


auto CharGen::NextDot() -> output_type
{
	return { 0 };
}
