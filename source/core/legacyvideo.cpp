#include <win32/winhvpx.hpp>

#include <core/machine.hpp>
#include <core/legacyvideo.hpp>

#include <utils/literals.hpp>

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
	auto source_buffer_v = win32::virtual_alloc_s(video_memory_v.Size(), 
		win32::page_protection_type::read_write);
	m_Machine.SuspendAllProcessors();
	video_memory_v.CopyDirtyPagesTo(source_buffer_v);

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////

	static constexpr const std::uint32_t palette_s[] = {
		0x00000000u, 0x000000AAu, 0x00AA0000u, 0x00AA00AAu,
		0x0000AA00u, 0x0000AAAAu, 0x00AA5500u, 0x00AAAAAAu,
		0x00555555u, 0x005555FFu, 0x00FF5555u, 0x00FF55FFu,
		0x0055FF55u, 0x0055FFFFu, 0x00FFFF55u, 0x00FFFFFFu
	};

	for (auto yy = 0u; yy < m_Height; ++yy) {
	for (auto xx = 0u; xx < m_Width;  ++xx) {
		auto const char_v = (std::uint8_t)source_buffer_v[2*(yy*80u + xx) + 0];
		auto const attr_v = (std::uint8_t)source_buffer_v[2*(yy*80u + xx) + 1];

		auto const color0_v = palette_s[(attr_v >> 0u)&0xFu];
		auto const color1_v = palette_s[(attr_v >> 4u)&0xFu];	
	}}

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////

  return { std::move(render_buffer_v), 16666us };
}
