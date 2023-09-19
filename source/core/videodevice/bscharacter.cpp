#include <core/videodevice/bscharacter.hpp>
#include <core/videodevice.hpp>

#include <array>

using core::videodevice::BsCharacter;

static inline constexpr auto glyph_size(core::videodevice::video_mode mode_v) noexcept
	-> std::array<std::uint8_t, 2u>
{
	using namespace core::videodevice;
	switch (mode_v)
	{
	case video_mode::character_color_8x8:  return { 0x08u, 0x08u };
	case video_mode::character_color_8x14: return { 0x08u, 0x0Eu };
	case video_mode::character_color_8x16: return { 0x08u, 0x10u };
	case video_mode::character_color_9x8:  return { 0x09u, 0x08u };
	case video_mode::character_color_9x14: return { 0x09u, 0x0Eu };
	case video_mode::character_color_9x16: return { 0x09u, 0x10u };
	case video_mode::character_mono_8x8:   return { 0x08u, 0x08u };
	case video_mode::character_mono_8x14:  return { 0x08u, 0x0Eu };
	case video_mode::character_mono_8x16:  return { 0x08u, 0x10u };
	case video_mode::character_mono_9x8:   return { 0x09u, 0x08u };
	case video_mode::character_mono_9x14:  return { 0x09u, 0x0Eu };
	case video_mode::character_mono_9x16:  return { 0x09u, 0x10u };
	default: return { 1u, 1u };
	}
}

static inline constexpr auto is_color_mode(core::videodevice::video_mode mode_v) noexcept
	-> bool
{
	using namespace core::videodevice;
	switch (mode_v)
	{
	case video_mode::character_color_8x8:
	case video_mode::character_color_8x14:
	case video_mode::character_color_8x16:
	case video_mode::character_color_9x8:
	case video_mode::character_color_9x14:
	case video_mode::character_color_9x16:
		return true;
	default: 
		return false;
	}
}

BsCharacter::BsCharacter(core::Machine& machine_v, core::VideoDevice& device_v, 
	uint16_t hsize_v, uint16_t vsize_v, video_mode mode_v, uint16_t flags_v)
	: BsCommon  { machine_v, device_v }
	, m_Mode		{ mode_v }
	, m_Cols		{ (uint16_t)(hsize_v / glyph_size(mode_v)[0]) }
	, m_Rows		{ (uint16_t)(vsize_v / glyph_size(mode_v)[1]) }
	, m_IsColor	{ is_color_mode(mode_v)}
{}

auto BsCharacter::Refresh(duration_type time_v) -> void {
	using namespace std::chrono_literals;
	using namespace std::chrono;
	auto const window_size_v = utils::round_ceil(m_Cols * m_Rows * 2u, 0x1000u);
	auto text_view_s = m_Device.GetMemoryRegion({ 0x0u, window_size_v });
	auto font_view_s = m_Device.GetMemoryRegion({ 0x20000u, 0x2000u });
	
}
