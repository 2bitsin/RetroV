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

static constexpr const std::uint8_t s_DefaultPalette[][3] = 
{
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x2A},
	{0x00, 0x2A, 0x00},
	{0x00, 0x2A, 0x2A},
	{0x2A, 0x00, 0x00},
	{0x2A, 0x00, 0x2A},
	{0x2A, 0x15, 0x00},
	{0x2A, 0x2A, 0x2A},
	{0x15, 0x15, 0x15},
	{0x15, 0x15, 0x3F},
	{0x15, 0x3F, 0x15},
	{0x15, 0x3F, 0x3F},
	{0x3F, 0x15, 0x15},
	{0x3F, 0x15, 0x3F},
	{0x3F, 0x3F, 0x15},
	{0x3F, 0x3F, 0x3F},
	{0x00, 0x00, 0x00},
	{0x05, 0x05, 0x05},
	{0x08, 0x08, 0x08},
	{0x0B, 0x0B, 0x0B},
	{0x0E, 0x0E, 0x0E},
	{0x11, 0x11, 0x11},
	{0x14, 0x14, 0x14},
	{0x18, 0x18, 0x18},
	{0x1C, 0x1C, 0x1C},
	{0x20, 0x20, 0x20},
	{0x24, 0x24, 0x24},
	{0x28, 0x28, 0x28},
	{0x2D, 0x2D, 0x2D},
	{0x32, 0x32, 0x32},
	{0x38, 0x38, 0x38},
	{0x3F, 0x3F, 0x3F},
	{0x00, 0x00, 0x3F},
	{0x10, 0x00, 0x3F},
	{0x1F, 0x00, 0x3F},
	{0x2F, 0x00, 0x3F},
	{0x3F, 0x00, 0x3F},
	{0x3F, 0x00, 0x2F},
	{0x3F, 0x00, 0x1F},
	{0x3F, 0x00, 0x10},
	{0x3F, 0x00, 0x00},
	{0x3F, 0x10, 0x00},
	{0x3F, 0x1F, 0x00},
	{0x3F, 0x2F, 0x00},
	{0x3F, 0x3F, 0x00},
	{0x2F, 0x3F, 0x00},
	{0x1F, 0x3F, 0x00},
	{0x10, 0x3F, 0x00},
	{0x00, 0x3F, 0x00},
	{0x00, 0x3F, 0x10},
	{0x00, 0x3F, 0x1F},
	{0x00, 0x3F, 0x2F},
	{0x00, 0x3F, 0x3F},
	{0x00, 0x2F, 0x3F},
	{0x00, 0x1F, 0x3F},
	{0x00, 0x10, 0x3F},
	{0x1F, 0x1F, 0x3F},
	{0x27, 0x1F, 0x3F},
	{0x2F, 0x1F, 0x3F},
	{0x37, 0x1F, 0x3F},
	{0x3F, 0x1F, 0x3F},
	{0x3F, 0x1F, 0x37},
	{0x3F, 0x1F, 0x2F},
	{0x3F, 0x1F, 0x27},
	{0x3F, 0x1F, 0x1F},
	{0x3F, 0x27, 0x1F},
	{0x3F, 0x2F, 0x1F},
	{0x3F, 0x37, 0x1F},
	{0x3F, 0x3F, 0x1F},
	{0x37, 0x3F, 0x1F},
	{0x2F, 0x3F, 0x1F},
	{0x27, 0x3F, 0x1F},
	{0x1F, 0x3F, 0x1F},
	{0x1F, 0x3F, 0x27},
	{0x1F, 0x3F, 0x2F},
	{0x1F, 0x3F, 0x37},
	{0x1F, 0x3F, 0x3F},
	{0x1F, 0x37, 0x3F},
	{0x1F, 0x2F, 0x3F},
	{0x1F, 0x27, 0x3F},
	{0x2D, 0x2D, 0x3F},
	{0x31, 0x2D, 0x3F},
	{0x36, 0x2D, 0x3F},
	{0x3A, 0x2D, 0x3F},
	{0x3F, 0x2D, 0x3F},
	{0x3F, 0x2D, 0x3A},
	{0x3F, 0x2D, 0x36},
	{0x3F, 0x2D, 0x31},
	{0x3F, 0x2D, 0x2D},
	{0x3F, 0x31, 0x2D},
	{0x3F, 0x36, 0x2D},
	{0x3F, 0x3A, 0x2D},
	{0x3F, 0x3F, 0x2D},
	{0x3A, 0x3F, 0x2D},
	{0x36, 0x3F, 0x2D},
	{0x31, 0x3F, 0x2D},
	{0x2D, 0x3F, 0x2D},
	{0x2D, 0x3F, 0x31},
	{0x2D, 0x3F, 0x36},
	{0x2D, 0x3F, 0x3A},
	{0x2D, 0x3F, 0x3F},
	{0x2D, 0x3A, 0x3F},
	{0x2D, 0x36, 0x3F},
	{0x2D, 0x31, 0x3F},
	{0x00, 0x00, 0x1C},
	{0x07, 0x00, 0x1C},
	{0x0E, 0x00, 0x1C},
	{0x15, 0x00, 0x1C},
	{0x1C, 0x00, 0x1C},
	{0x1C, 0x00, 0x15},
	{0x1C, 0x00, 0x0E},
	{0x1C, 0x00, 0x07},
	{0x1C, 0x00, 0x00},
	{0x1C, 0x07, 0x00},
	{0x1C, 0x0E, 0x00},
	{0x1C, 0x15, 0x00},
	{0x1C, 0x1C, 0x00},
	{0x15, 0x1C, 0x00},
	{0x0E, 0x1C, 0x00},
	{0x07, 0x1C, 0x00},
	{0x00, 0x1C, 0x00},
	{0x00, 0x1C, 0x07},
	{0x00, 0x1C, 0x0E},
	{0x00, 0x1C, 0x15},
	{0x00, 0x1C, 0x1C},
	{0x00, 0x15, 0x1C},
	{0x00, 0x0E, 0x1C},
	{0x00, 0x07, 0x1C},
	{0x0E, 0x0E, 0x1C},
	{0x11, 0x0E, 0x1C},
	{0x15, 0x0E, 0x1C},
	{0x18, 0x0E, 0x1C},
	{0x1C, 0x0E, 0x1C},
	{0x1C, 0x0E, 0x18},
	{0x1C, 0x0E, 0x15},
	{0x1C, 0x0E, 0x11},
	{0x1C, 0x0E, 0x0E},
	{0x1C, 0x11, 0x0E},
	{0x1C, 0x15, 0x0E},
	{0x1C, 0x18, 0x0E},
	{0x1C, 0x1C, 0x0E},
	{0x18, 0x1C, 0x0E},
	{0x15, 0x1C, 0x0E},
	{0x11, 0x1C, 0x0E},
	{0x0E, 0x1C, 0x0E},
	{0x0E, 0x1C, 0x11},
	{0x0E, 0x1C, 0x15},
	{0x0E, 0x1C, 0x18},
	{0x0E, 0x1C, 0x1C},
	{0x0E, 0x18, 0x1C},
	{0x0E, 0x15, 0x1C},
	{0x0E, 0x11, 0x1C},
	{0x14, 0x14, 0x1C},
	{0x16, 0x14, 0x1C},
	{0x18, 0x14, 0x1C},
	{0x1A, 0x14, 0x1C},
	{0x1C, 0x14, 0x1C},
	{0x1C, 0x14, 0x1A},
	{0x1C, 0x14, 0x18},
	{0x1C, 0x14, 0x16},
	{0x1C, 0x14, 0x14},
	{0x1C, 0x16, 0x14},
	{0x1C, 0x18, 0x14},
	{0x1C, 0x1A, 0x14},
	{0x1C, 0x1C, 0x14},
	{0x1A, 0x1C, 0x14},
	{0x18, 0x1C, 0x14},
	{0x16, 0x1C, 0x14},
	{0x14, 0x1C, 0x14},
	{0x14, 0x1C, 0x16},
	{0x14, 0x1C, 0x18},
	{0x14, 0x1C, 0x1A},
	{0x14, 0x1C, 0x1C},
	{0x14, 0x1A, 0x1C},
	{0x14, 0x18, 0x1C},
	{0x14, 0x16, 0x1C},
	{0x00, 0x00, 0x10},
	{0x04, 0x00, 0x10},
	{0x08, 0x00, 0x10},
	{0x0C, 0x00, 0x10},
	{0x10, 0x00, 0x10},
	{0x10, 0x00, 0x0C},
	{0x10, 0x00, 0x08},
	{0x10, 0x00, 0x04},
	{0x10, 0x00, 0x00},
	{0x10, 0x04, 0x00},
	{0x10, 0x08, 0x00},
	{0x10, 0x0C, 0x00},
	{0x10, 0x10, 0x00},
	{0x0C, 0x10, 0x00},
	{0x08, 0x10, 0x00},
	{0x04, 0x10, 0x00},
	{0x00, 0x10, 0x00},
	{0x00, 0x10, 0x04},
	{0x00, 0x10, 0x08},
	{0x00, 0x10, 0x0C},
	{0x00, 0x10, 0x10},
	{0x00, 0x0C, 0x10},
	{0x00, 0x08, 0x10},
	{0x00, 0x04, 0x10},
	{0x08, 0x08, 0x10},
	{0x0A, 0x08, 0x10},
	{0x0C, 0x08, 0x10},
	{0x0E, 0x08, 0x10},
	{0x10, 0x08, 0x10},
	{0x10, 0x08, 0x0E},
	{0x10, 0x08, 0x0C},
	{0x10, 0x08, 0x0A},
	{0x10, 0x08, 0x08},
	{0x10, 0x0A, 0x08},
	{0x10, 0x0C, 0x08},
	{0x10, 0x0E, 0x08},
	{0x10, 0x10, 0x08},
	{0x0E, 0x10, 0x08},
	{0x0C, 0x10, 0x08},
	{0x0A, 0x10, 0x08},
	{0x08, 0x10, 0x08},
	{0x08, 0x10, 0x0A},
	{0x08, 0x10, 0x0C},
	{0x08, 0x10, 0x0E},
	{0x08, 0x10, 0x10},
	{0x08, 0x0E, 0x10},
	{0x08, 0x0C, 0x10},
	{0x08, 0x0A, 0x10},
	{0x0B, 0x0B, 0x10},
	{0x0C, 0x0B, 0x10},
	{0x0D, 0x0B, 0x10},
	{0x0F, 0x0B, 0x10},
	{0x10, 0x0B, 0x10},
	{0x10, 0x0B, 0x0F},
	{0x10, 0x0B, 0x0D},
	{0x10, 0x0B, 0x0C},
	{0x10, 0x0B, 0x0B},
	{0x10, 0x0C, 0x0B},
	{0x10, 0x0D, 0x0B},
	{0x10, 0x0F, 0x0B},
	{0x10, 0x10, 0x0B},
	{0x0F, 0x10, 0x0B},
	{0x0D, 0x10, 0x0B},
	{0x0C, 0x10, 0x0B},
	{0x0B, 0x10, 0x0B},
	{0x0B, 0x10, 0x0C},
	{0x0B, 0x10, 0x0D},
	{0x0B, 0x10, 0x0F},
	{0x0B, 0x10, 0x10},
	{0x0B, 0x0F, 0x10},
	{0x0B, 0x0D, 0x10},
	{0x0B, 0x0C, 0x10},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00}
};

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

auto VideoDevice::ConfigureRegisters(Configuration const& config_v) -> void {
	std::memset(&m_GCreg, 0, sizeof(m_GCreg));
	std::memset(&m_SQreg, 0, sizeof(m_SQreg));
}

auto VideoDevice::SetLegacyMapping(MemoryWindow target_v, std::size_t offset_v) -> void
{
	static constexpr utils::region64_type s_MemoryWindow[] = {
		{ utils::from_range, 0xA0000u, 0xC0000u },
		{ utils::from_range, 0xA0000u, 0xB0000u },	
		{ utils::from_range, 0xB0000u, 0xB8000u },
		{ utils::from_range, 0xB8000u, 0xC0000u }
	};

	auto const target_idx = std::to_underlying(target_v);
	if (target_idx < 0) {
		m_MappedMemory.clear();
		return;
	}

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
	ConfigureRegisters(config_v);
	SetLegacyMapping(kTextUpper32k, 0u);
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
	std::int32_t status_v{ 0 };
	if (data_v.size() != 1u) 
	{		
		if(data_v.size() > 0u) status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 0u, data_v.subspan(0u, 1u)); if (status_v != S_OK) return status_v;
		if(data_v.size() > 1u) status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 1u, data_v.subspan(1u, 1u)); if (status_v != S_OK) return status_v;
		if(data_v.size() > 2u) status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 2u, data_v.subspan(2u, 1u)); if (status_v != S_OK) return status_v;
		if(data_v.size() > 3u) status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 3u, data_v.subspan(3u, 1u)); if (status_v != S_OK) return status_v;
	}

	if (!is_write_v) 
	{
		auto const [status_v, value_v] = IoPortFetch(vcpu_v, port_v);
		if (status_v != S_OK) 
			return status_v;
		if (!data_v.write(value_v))
			return ERROR_ACCESS_DENIED;		
		return ERROR_SUCCESS;
	}
	return IoPortWrite(vcpu_v, port_v, data_v.as<std::uint8_t>());
}

static constexpr auto const Q = []<class T>(T port_v) constexpr -> T { return port_v - 0x3B0u; };

auto VideoDevice::IoPortWrite(Processor const& vcpu_v, std::uint16_t port_v, std::uint8_t value_v) -> std::int32_t
{
	switch (port_v) 
	{
	case Q(0x3CEu): m_GCreg.index = value_v&7u; return ERROR_SUCCESS;	
	case Q(0x3CFu): 
		switch (m_GCreg.index)
		{
		case 0: m_GCreg.set_reset.bits        = value_v; return ERROR_SUCCESS;
		case 1: m_GCreg.enable_set_reset.bits = value_v; return ERROR_SUCCESS;
		case 2: m_GCreg.color_compare.bits    = value_v; return ERROR_SUCCESS;
		case 3: m_GCreg.data_rotate.bits      = value_v; return ERROR_SUCCESS;
		case 4: m_GCreg.read_map_select.bits  = value_v; return ERROR_SUCCESS;
		case 5: m_GCreg.mode.bits             = value_v; return ERROR_SUCCESS;
		case 6: m_GCreg.misc.bits             = value_v; return ERROR_SUCCESS;
		case 7: m_GCreg.compare_enable.bits   = value_v; return ERROR_SUCCESS;
		default:
			__debugbreak();
			return ERROR_SUCCESS;
		}
		break;
	case Q(0x3C4u): m_SQreg.index = value_v&7u; return ERROR_SUCCESS;
	case Q(0x3C5u):
		switch (m_SQreg.index)
		{
		case 0: m_SQreg.reset.bits      = value_v; return ERROR_SUCCESS;
		case 1: m_SQreg.clock.bits      = value_v; return ERROR_SUCCESS;
		case 2: m_SQreg.write_mask.bits = value_v; return ERROR_SUCCESS;
		case 3: m_SQreg.char_map.bits   = value_v; return ERROR_SUCCESS;
		case 4: m_SQreg.mem_mode.bits   = value_v; return ERROR_SUCCESS;
		default: 
			__debugbreak();
			return ERROR_SUCCESS;		
		}
	default: return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}

auto VideoDevice::IoPortFetch(Processor const& vcpu_v, std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>
{
	switch (port_v)
	{
	case Q(0x3CEu):
		return { ERROR_SUCCESS, m_GCreg.index };
	case Q(0x3CFu):
		switch (m_GCreg.index)
		{
		case 0: return { ERROR_SUCCESS, m_GCreg.set_reset.bits };
		case 1: return { ERROR_SUCCESS, m_GCreg.enable_set_reset.bits };
		case 2: return { ERROR_SUCCESS, m_GCreg.color_compare.bits };
		case 3: return { ERROR_SUCCESS, m_GCreg.data_rotate.bits };
		case 4: return { ERROR_SUCCESS, m_GCreg.read_map_select.bits };
		case 5: return { ERROR_SUCCESS, m_GCreg.mode.bits };
		case 6: return { ERROR_SUCCESS, m_GCreg.misc.bits };
		case 7: return { ERROR_SUCCESS, m_GCreg.compare_enable.bits };
		default: return { ERROR_SUCCESS, 0u };
		}
		break;
	case Q(0x3C4):
		return { ERROR_SUCCESS, m_SQreg.index };
	case Q(0x3C5):
		switch (m_SQreg.index)
		{
		case 0: return { ERROR_SUCCESS, m_SQreg.reset.bits };
		case 1: return { ERROR_SUCCESS, m_SQreg.clock.bits };
		case 2: return { ERROR_SUCCESS, m_SQreg.write_mask.bits };
		case 3: return { ERROR_SUCCESS, m_SQreg.char_map.bits };
		case 4: return { ERROR_SUCCESS, m_SQreg.mem_mode.bits };
		default: return { ERROR_SUCCESS, 0u };
		}
		break;
	default:
		__debugbreak();
		return { ERROR_SUCCESS, 0u };
	}
	return { ERROR_SUCCESS, 0u };
}

auto VideoDevice::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	__debugbreak();
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

			auto const& color0_v = s_DefaultPalette[(attr_v >> 4u) & 0xFu];
			auto const& color1_v = s_DefaultPalette[(attr_v >> 0u) & 0xFu];

			auto const& color_v = ((glyph_v >> (7 - (xx % 8u))) & 1u) ? color1_v : color0_v;		
			view_v[yy][xx] = 0xFF000000u + ((color_v[0] * 0x100u + color_v[1]) * 0x100u + color_v[2]);
		}

	////////////////////////////////////////
	// 
	//	Temporary code to render 80 col text
	//
	////////////////////////////////////////	
	return duration_cast<duration_type>(1s / 60.0);
}

