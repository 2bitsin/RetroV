#include <core/videodevice.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <utils/validate.hpp>
#include <utils/literals.hpp>
#include <utils/surface.hpp>
#include <utils/lambda.hpp>
#include <utils/paths.hpp>

#include <device/resources/font.hpp>

#include <algorithm>
#include <chrono>
#include <ranges>

using core::VideoDevice;

VideoDevice::VideoDevice(core::Machine& machine_v)
	: m_Machine{ machine_v }
	, m_Height{ 0 }
	, m_Width{ 0 }
{}

VideoDevice::~VideoDevice()
{
	Stop();
}


auto VideoDevice::ConfigureBiosROM(Configuration const& config_v) -> void
{
	using namespace size_literals;
	using namespace win32;

	auto const video_rom_path_v = utils::build_path(config_v.GetPropertyString("video.bios.path"));
	utils::validate_binary(video_rom_path_v, 4_KiB, 1u, 16u);
	auto const size_v = std::filesystem::file_size(video_rom_path_v);
	auto const where_v = utils::region64_type{ utils::from_range, 0xD0000u - size_v, 0xD0000u };
	m_MappedRomFile.emplace(MappedFile(video_rom_path_v, {}, open_existing, execute_write_copy));
	m_MappedRomRange.emplace(m_Machine.GetPartition(), where_v , kAccessDevice, m_MappedRomFile.value());
}

auto VideoDevice::ConfigureMemory(Configuration const& config_v) -> void
{
	using namespace win32;
	using namespace size_literals;
	auto const memory_kilobytes_v = config_v.GetPropertyUint64("video.memory.kilobytes");
	m_VideoMemory[0] = VirtualAlloc_s(memory_kilobytes_v * 1_KiB, page_prot::read_write, alloc_flag::commit | alloc_flag::reserve | alloc_flag::write_watch);
	m_VideoMemory[1] = VirtualAlloc_s(memory_kilobytes_v * 1_KiB, page_prot::read_write);
}

auto VideoDevice::SetLegacyMapping(MemoryWindow target_v, std::size_t offset_v) -> void
{
	static constexpr utils::region64_type s_MemoryWindow[] = {
		{ utils::from_range, 0xA0000u, 0xB0000u },
		{ utils::from_range, 0xB0000u, 0xB8000u },
		{ utils::from_range, 0xB8000u, 0xC0000u }
	};

	auto const target_idx = std::to_underlying(target_v);
	if (target_idx >= std::size(s_MemoryWindow)) {
		throw std::out_of_range("Invalid target window");
	}

	auto const& window_v = s_MemoryWindow[target_idx];

	offset_v *= kPageSize;

	if (offset_v >= m_VideoMemory[0].size()) {
		throw std::out_of_range("Invalid page value");
	}

	m_MappedMemory.clear();
	m_MappedMemory.emplace_back(m_Machine.GetPartition(), window_v, kAccessDevice, m_VideoMemory[0]
		.subspan(offset_v, window_v.size()));
}

auto VideoDevice::Initialize(Configuration const& config_v) -> void
{
	using enum MemoryWindow;

	using namespace win32;
	using namespace size_literals;

	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);

	SetLegacyMapping(kTextUpper, 0u);

	m_Height = 400u;
	m_Width = 640u;

}

auto VideoDevice::RefreshThread(std::stop_token stoppee_v) -> void
{
	using namespace std::chrono_literals;
	using namespace std::chrono;

	std::mutex mutex_v;
	std::unique_lock lock_v{ mutex_v };
	std::condition_variable timer_v;
	std::stop_callback stcbk_v(stoppee_v, [&timer_v]() {
		timer_v.notify_all();
	});

	auto const stop_requested_q = [&stoppee_v]() {
		return stoppee_v.stop_requested();
	};

	auto next_frame_v = steady_clock::now();
	auto& display_v = m_Machine.GetDisplay();
	while (!stoppee_v.stop_requested()) {
		auto surface_v = display_v.AcquireSurface(m_Width, m_Height);
		auto const delta_time_v = Refresh(surface_v);
		display_v.Present(std::move(surface_v));
		next_frame_v += delta_time_v;
		if (timer_v.wait_until(lock_v, next_frame_v, stop_requested_q)) {
			break;
		}
	}
}

auto VideoDevice::Start() -> void
{
	m_Refresh = std::async(std::launch::async,
		utils::lambda(this, &VideoDevice::RefreshThread),
		m_Stopper.get_token());
}

auto VideoDevice::Stop() -> void
{
	if (m_Refresh.valid()) {
		m_Stopper.request_stop();
		m_Refresh.wait();
	}
}

auto VideoDevice::Restart() -> void
{}

auto VideoDevice::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	static constexpr auto const Q = [] (std::uint16_t port_v) constexpr -> std::uint16_t {
		return port_v - 0x3B0u;
	};

	switch (port_v)
	{
	case Q(0x3C4u): __debugbreak(); break;		
	case Q(0x3C5u): __debugbreak(); break;
	case Q(0x3CEu): __debugbreak(); break;

	}
	__debugbreak();
	return S_OK;
}

auto VideoDevice::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	return S_OK;
}

auto VideoDevice::Refresh(Display::surface_tmp& surface_v) -> duration_type
{
	using namespace std::chrono_literals;
	using namespace size_literals;
	using namespace std::chrono;

	m_Machine.SuspendAllProcessors();
	win32::CopyDirtyPages(m_VideoMemory[0], m_VideoMemory[1]);
	m_Machine.ResumeAllProcessors();

	::SDL_FillRect(surface_v.get(), nullptr, 0xFFFF0000u);

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

	utils::surface_view<std::uint32_t> view_v{ surface_v.get() };

	auto const chars_v = utils::as_span_of<std::uint16_t>(m_VideoMemory[1]);	

	for (auto yy = 0u; yy < m_Height; ++yy)
		for (auto xx = 0u; xx < m_Width; ++xx)
		{
			auto const y = yy / 16u;
			auto const x = xx / 8u;

			auto const& cell_v = chars_v[y * 80u + x];
			auto char_v = (cell_v >> 0) & 0xFFu;
			auto attr_v = (cell_v >> 8) & 0xFFu;

			auto&& font_v = device::resources::font::get_8x16();

			auto const glyph_v = (std::uint8_t)font_v.data[char_v * font_v.rows + (yy % 16)];

			auto const color0_v = palette_s[(attr_v >> 4u) & 0xFu];
			auto const color1_v = palette_s[(attr_v >> 0u) & 0xFu];

			auto const color_v = ((glyph_v >> (7 - (xx % 8u))) & 1u) ? color1_v : color0_v;

			view_v[yy][xx] = color_v;
		}

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////	
	return duration_cast<duration_type>(1s / 60.0);
}

