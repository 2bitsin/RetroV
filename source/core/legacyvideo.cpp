#include <win32/winhvpx.hpp>

#include <core/machine.hpp>
#include <core/legacyvideo.hpp>

#include <utils/literals.hpp>

#include <device/resources/font.hpp>

#include <ranges>
#include <algorithm>

using core::LegacyVideo;

LegacyVideo::LegacyVideo(Machine& machine_v)
	: m_Machine{ machine_v }
	, m_MonoTextWindow{ std::nullopt }
	, m_ColorTextWindow{ std::nullopt }
	, m_GraphicsWindow{ std::nullopt }
	, m_Height{ 0 }
	, m_Width{ 0 }
{}

auto LegacyVideo::Initialize() -> std::int32_t
{
	using namespace size_literals;

	m_MonoTextWindow.emplace(m_Machine.GetPartition(), 0xB0000u, 0x8000u, kAccessDevice|kTrackDirty);
	m_ColorTextWindow.emplace(m_Machine.GetPartition(), 0xB8000u, 0x8000u, kAccessDevice|kTrackDirty);
	m_TemporaryBuffer = win32::virtual_alloc_s(0x10000u, win32::page_protection_type::read_write);
	m_Height = 400u;
	m_Width = 640u;
	return S_OK;
}

auto LegacyVideo::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	return S_OK;
}

auto LegacyVideo::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	return S_OK;
}

auto LegacyVideo::Render() -> std::tuple<utils::buffer2d<std::uint32_t>, std::chrono::microseconds>
{
	using namespace size_literals;
	using namespace std::chrono_literals;

	if (!m_ColorTextWindow.has_value()) 
		throw std::runtime_error{ "Video buffer not present!" };	
	auto& video_memory_v = *m_ColorTextWindow;
	utils::buffer2d<std::uint32_t> render_buffer_v { m_Width, m_Height };	
	m_Machine.SuspendAllProcessors();
	//WIN32_ERROR_ASSERT(video_memory_v.CopyDirtyPagesTo(m_TemporaryBuffer));
	std::ranges::copy(video_memory_v.Data(), m_TemporaryBuffer.begin());
	m_Machine.ResumeAllProcessors();


	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////

	static constexpr const std::uint32_t palette_s[] = {
		0xFF000000u, 0xFF0000AAu, 0xFFAA0000u, 0xFFAA00AAu,
		0xFF00AA00u, 0xFF00AAAAu, 0xFFAA5500u, 0xFFAAAAAAu,
		0xFF555555u, 0xFF5555FFu, 0xFFFF5555u, 0xFFFF55FFu,
		0xFF55FF55u, 0xFF55FFFFu, 0xFFFFFF55u, 0xFFFFFFFFu
	};

	for (auto yy = 0u; yy < m_Height; ++yy) 
	for (auto xx = 0u; xx < m_Width;  ++xx) 
	{
		auto const y = yy/16u;
		auto const x = xx/8u;

		auto char_v = (std::uint8_t)m_TemporaryBuffer[2*(y*80u + x) + 0];
		auto attr_v = (std::uint8_t)m_TemporaryBuffer[2*(y*80u + x) + 1];

		auto&& font_v = device::resources::font::get_8x16();

		auto const glyph_v = (std::uint8_t)font_v.data[char_v*font_v.rows + (yy%16)];
		if (x||y)__debugbreak();

		auto const color0_v = palette_s[(attr_v >> 4u)&0xFu];	
		auto const color1_v = palette_s[(attr_v >> 0u)&0xFu];

		render_buffer_v[{xx, yy}] = (glyph_v & (1u << (xx % 8u))) ? color1_v : color0_v;
	}

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////


  return { std::move(render_buffer_v), 16666us };
}
