#include <win32/winhvpx.hpp>

#include <core/machine.hpp>
#include <core/legacyvideo.hpp>

#include <utils/literals.hpp>
#include <utils/surface.hpp>
#include <utils/lambda.hpp>

#include <device/resources/font.hpp>

#include <algorithm>
#include <chrono>
#include <ranges>

using core::LegacyVideo;

LegacyVideo::LegacyVideo(Machine& machine_v)
	: m_Machine{ machine_v }
	, m_Height{ 0 }
	, m_Width{ 0 }
{}

auto LegacyVideo::Initialize() -> void
{
	using namespace win32;
	using namespace size_literals;
	m_Height = 400u;
	m_Width = 640u;
	m_VideoMemory = VirtualAlloc_s(0x10000u, page_prot::read_write, alloc_flag::commit|alloc_flag::reserve|alloc_flag::write_watch);
	m_BackBuffer = VirtualAlloc_s(0x10000u, page_prot::read_write);
	m_MappedRanges.emplace_back(m_Machine.GetPartition(), s_MemoryWindow[1u], kAccessDevice, m_VideoMemory);
}

auto LegacyVideo::RefreshThread(std::stop_token stoppee_v) -> void 
{
	using namespace std::chrono_literals;
	using namespace std::chrono;

	std::mutex mutex_v;
	std::unique_lock lock_v{ mutex_v };
	std::condition_variable timer_v;
	std::stop_callback stcbk_v(stoppee_v, [&timer_v] () { 
		timer_v.notify_all();
	});
	auto next_frame_v = steady_clock::now();
	auto& display_v = m_Machine.GetDisplay();
	while (!stoppee_v.stop_requested()) {
		auto surface_v = display_v.AcquireSurface(m_Width, m_Height);
		auto const delta_time_v = Refresh(surface_v);
		display_v.Present(std::move(surface_v));
		next_frame_v += delta_time_v;
		if (std::cv_status::timeout !=
			timer_v.wait_until(lock_v, next_frame_v)) {
			break;
		}
	}
}

auto LegacyVideo::Start() -> void
{
	m_Refresh = std::async(std::launch::async, 
		utils::lambda(this, &LegacyVideo::RefreshThread), 
		m_Stopper.get_token());
}

auto LegacyVideo::Stop() -> void
{
	if (m_Refresh.valid()) {
		m_Stopper.request_stop();		
		m_Refresh.wait();
	}
}

auto LegacyVideo::Restart() -> void
{}

auto LegacyVideo::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	return S_OK;
}

auto LegacyVideo::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	return S_OK;
}

auto LegacyVideo::Refresh(Display::surface_tmp& surface_v) -> duration_type
{
	using namespace std::chrono_literals;
	using namespace size_literals;
	using namespace std::chrono;
	
	m_Machine.SuspendAllProcessors();	
	win32::CopyDirtyPages(m_BackBuffer, m_VideoMemory);
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

		auto char_v = (std::uint8_t)m_BackBuffer[2*(y*80u + x) + 0];
		auto attr_v = (std::uint8_t)m_BackBuffer[2*(y*80u + x) + 1];

		auto&& font_v = device::resources::font::get_8x16();

		auto const glyph_v = (std::uint8_t)font_v.data[char_v*font_v.rows + (yy%16)];

		auto const color0_v = palette_s[(attr_v >> 4u)&0xFu];	
		auto const color1_v = palette_s[(attr_v >> 0u)&0xFu];

		auto const color_v = ((glyph_v >> (7 - (xx % 8u))) & 1u) ? color1_v : color0_v;
	}

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////	
	return duration_cast<duration_type>(1s / 60.0);
}
