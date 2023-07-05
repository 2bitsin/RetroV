#include <device/vgadevice.hpp>
#include <device/vgadevice/font.hpp>
#include <core/hypervisor.hpp>
#include <utils/literals.hpp>

#include <functional>
#include <stdexcept>
#include <format>

static inline constexpr std::uint16_t const kFetchPorts[] = {
	0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
	0x03C8, 0x03C9, 0x03CA, 0x03CC, 0x03CE, 0x03CF, 0x03D0, 0x03D1,
	0x03D2, 0x03D3, 0x03D4, 0x03D5, 0x03D6, 0x03DA
};

static inline constexpr std::uint16_t const kWritePorts[] = {
	0x03C0, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8,
	0x03C9, 0x03CE, 0x03CF, 0x03D0, 0x03D1, 0x03D2, 0x03D3, 0x03D4,
	0x03D5, 0x03D6
};

static inline constexpr std::uint32_t const kTextColorPalette[] = {
	0x00000000, 0x000000AA, 0x0000AA00, 0x0000AAAA,
	0x00AA0000, 0x00AA00AA, 0x00AA5500, 0x00AAAAAA,
	0x00555555, 0x005555FF, 0x0055FF55, 0x0055FFFF,
	0x00FF5555, 0x00FF55FF, 0x00FFFF55, 0x00FFFFFF
};

using device::VGADevice;
using device::Config;

VGADevice::VGADevice(core::Hypervisor& hypervisor_v, Config const& config_v)
	:	m_Hypervisor(&hypervisor_v)
	, m_Window(nullptr)
{
	if (nullptr == m_Window) {
		m_Window = SDL_CreateWindow("SimpleVGA", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			640*2u, 400 * 2u,
			SDL_WINDOW_SHOWN);
		if (nullptr == m_Window) {
			throw std::runtime_error(std::format("{}: {}\n",
				__func__, SDL_GetError()));
		}
	}
	
	/*
	auto& ioman_v = m_Hypervisor->GetIoManager();
	for (auto const& port_v : kFetchPorts) ioman_v.RegisterFetchCallback(port_v, 
		[this] (auto& hypervisor_v, auto& processor_v, auto port_v, auto& data_v, auto size_v) -> bool {
			//return IoFetch(hypervisor_v, processor_v, port_v, data_v, size_v);
			return false;
		});
	for (auto const& port_v : kWritePorts) ioman_v.RegisterWriteCallback(port_v, 
		[this] (auto& hypervisor_v, auto& processor_v, auto port_v, auto data_v, auto size_v) -> bool {
			//return IoWrite(hypervisor_v, processor_v, port_v, data_v, size_v);
			return false;
		});
	*/
	SetVideoMode(RenderingMode::kTextColor, 80u, 25u);
}

VGADevice::~VGADevice()
{
	using namespace size_literals;

	if (m_TaskIndex) {
		auto& sched_v = m_Hypervisor->GetScheduler();
		sched_v.DeviceStop(m_TaskIndex);
		m_TaskIndex = 0u;
	}

	auto& mman_v = m_Hypervisor->GetMemManager();
	mman_v.UnmapPhysical(0xA0000u, 64_KiB);
	mman_v.UnmapPhysical(0xB0000u, 64_KiB);

	if (m_VramBlock) {
		auto& pool_v = m_Hypervisor->GetMemPool();
		pool_v.FreeBlock(m_VramBlock);
	}

	if (nullptr != m_Window) {
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}
}

auto VGADevice::Emulate(core::Scheduler& scheduler_v, core::Service& service_v) -> void
{
	using namespace std::chrono_literals;
	using namespace std::chrono;

	auto ticks_next_v = high_resolution_clock::now();
	while(!service_v.StopRequested())
	{
		auto const ticks_now_v = high_resolution_clock::now();
		if (ticks_now_v < ticks_next_v) {
			std::this_thread::sleep_until(ticks_next_v);
		}

		auto& surface_v = *::SDL_GetWindowSurface(m_Window);
		::SDL_LockSurface(&surface_v);
		std::span surface_s { 
			(std::uint32_t*)surface_v.pixels, 
			std::size_t(surface_v.w * surface_v.h)
		};

		auto& pool_v = m_Hypervisor->GetMemPool();
		auto vram_s = utils::mutable_span_as<std::uint16_t>(
			pool_v.GetBlockView(m_VramBlock));
		
		for (auto& vram_w: vram_s) vram_w = (std::rand()&0xFFu)*0x100u + (std::rand()&0xFFu);
		auto font_s = VGAFont8x16();
		for (auto yy = 0u; yy < surface_v.h; yy += 1u)
		for (auto xx = 0u; xx < surface_v.w; xx += 1u) {
			auto const ty = (yy/2) / 0x10u; auto const dy = (yy/2) % 0x10u;
			auto const tx = (xx/2) / 0x08u; auto const dx = (xx/2) % 0x08u;			
			auto const cell_v = vram_s[ty * m_Width + tx];
			auto const fg_color_v = kTextColorPalette[(cell_v >> 0x8u) & 0x0F];
			auto const bg_color_v = kTextColorPalette[(cell_v >> 0xCu) & 0x0F];
			auto const ch_index_v = cell_v & 0xFFu;			
			auto const chr_bits_v = (std::uint8_t)font_s[ch_index_v*16u + dy];
			auto const chr_color_v = (chr_bits_v >> (8u - dx)) & 0x01u ? fg_color_v : bg_color_v;
			surface_s[yy * surface_v.w + xx] = chr_color_v;			
		}
		::SDL_UnlockSurface(&surface_v);	
		::SDL_UpdateWindowSurface(m_Window);		
	}
}

auto VGADevice::GetCategory() const noexcept -> device::DeviceCatory { 
	return DeviceCatory::kVideo; 
}

auto VGADevice::Pause() -> void
{
	(*m_Hypervisor).GetScheduler().DevicePause(m_TaskIndex);
}

auto VGADevice::Resume() -> void
{
	(*m_Hypervisor).GetScheduler().DeviceResume(m_TaskIndex);
}

auto VGADevice::SetVideoMode(RenderingMode mode_v, std::uint16_t width_v, std::uint16_t height_v) -> void
{
	using namespace size_literals;

	auto& sched_v = (*m_Hypervisor).GetScheduler();

	if (m_TaskIndex != 0u) {
		sched_v.DeviceStop(m_TaskIndex);}

	if (mode_v != RenderingMode::kTextColor) {
		throw std::runtime_error(std::format("{}: unsupported rendering mode\n", __func__));
	}
	if (width_v != 80 || height_v != 25) {
		throw std::runtime_error(std::format("{}: unsupported resolution\n", __func__));
	}

	m_RenderingMode = mode_v;
	m_Height = height_v;
	m_Width = width_v;

	SDL_SetWindowSize(m_Window, 8u*m_Width*2u, 16u*m_Height*2u);

	auto& pool_v = m_Hypervisor->GetMemPool();
	auto& mman_v = m_Hypervisor->GetMemManager();

	if (m_VramBlock != 0) {
		mman_v.UnmapPhysical(0xA0000u, 64_KiB);
		mman_v.UnmapPhysical(0xB8000u, 32_KiB);
		pool_v.FreeBlock(m_VramBlock);
		m_VramBlock = 0;
	}

	m_VramBlock = pool_v.AllocateBlock(32_KiB);
	mman_v.MapPhysical(m_VramBlock, 0xB8000u, 32_KiB, mman_v.kMemoryFlagsDevice);
	m_TaskIndex = sched_v.DeviceStart(*this);
	sched_v.DeviceResume(m_TaskIndex);
}

auto VGADevice::IoWrite(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t data_v, std::uint8_t size_v) -> bool
{
	return false;
}

auto VGADevice::IoFetch(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v) -> bool
{
	return false;
}
