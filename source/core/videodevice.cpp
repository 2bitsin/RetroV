#include <core/videodevice.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <utils/logger.hpp>
#include <utils/algorithm.hpp>
#include <utils/validate.hpp>
#include <utils/literals.hpp>
#include <utils/surface.hpp>
#include <utils/lambda.hpp>
#include <utils/paths.hpp>

#include <win32/whvcapabilities.hpp>
#include <win32/waitabletimer.hpp>

#include <bios/vmcall.h>

#include <algorithm>
#include <chrono>
#include <ranges>

using core::VideoDevice;
using core::VideoDeviceStateVga;

VideoDevice::VideoDevice(core::Machine& machine_v)
	: m_Machine{ machine_v }
{}

VideoDevice::~VideoDevice()
{
	Stop();
}

auto VideoDevice::Initialize(Configuration const& config_v) -> void
{
	using namespace win32;
	using namespace size_literals;

	ConfigureROM(config_v);
	ConfigureMemory(config_v);
}

auto VideoDevice::Start() -> void
{
}

auto VideoDevice::Stop() -> void
{
}

auto VideoDevice::Restart() -> void
{
	Stop();
	Start();
}

auto VideoDevice::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{	
	if (data_v.size() > 1u)
	{
		std::int32_t status_v{ 0 };

		if (data_v.size() > 0u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 0u, data_v.subspan(0u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() > 1u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 1u, data_v.subspan(1u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() > 2u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 2u, data_v.subspan(2u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() > 3u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 3u, data_v.subspan(3u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		return ERROR_SUCCESS;
	}

	if (is_write_v) 
		return m_State[0u].IoPortWrite(port_v, data_v.as<std::uint8_t>());
	auto const [status_v, value_v] = m_State[0u].IoPortFetch(port_v);
	if (status_v != ERROR_SUCCESS) 
		return status_v;
	data_v.write(value_v);
	return ERROR_SUCCESS;
}

auto VideoDevice::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> std::int32_t
{
	__debugbreak();
	return ERROR_SUCCESS;
}

auto VideoDevice::Hypercall(Processor const& vcpu_v, HypercallContext const& context_v) -> std::int32_t
{	
	using namespace win32::regs;

	auto const& hypercall_v = context_v.Hypercall;
	auto const& vpcontext_v = context_v.VpContext;

	switch (context_v.Function) {
	case HYPERCALL_VIDEO_BEGIN_UPDATE:
		return ERROR_SUCCESS;
	case HYPERCALL_VIDEO_END_UPDATE:
		m_State[0].Log();
		return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}

auto VideoDevice::ConfigureROM(core::Configuration const& config_v) -> void
{
	auto const path_v = config_v.GetPropertyString("video.rom.path");
	auto const validate_v = RomImage::validate{
		0x1000u, 0x01u, 0x10u };
	auto const region_v = RomImage::region_type{
		utils::from_range, 0xC0000u, 0xD0000u };
	auto const options_v = 0u;
	m_BiosRom.emplace(m_Machine.GetPartition(),
		validate_v, path_v, region_v, options_v);
}

auto VideoDevice::ConfigureMemory(core::Configuration const& config_v) -> void
{
	using namespace win32;
	using namespace size_literals;
	auto const size_bytes_v = config_v.GetPropertyUint64("video.memory.size.kilobytes")*1_KiB;
	m_VideoMemory[0u] = VirtualAlloc_s(size_bytes_v, read_write, commit|reserve|write_watch, nullptr);
	m_VideoMemory[1u] = VirtualAlloc_s(size_bytes_v, read_write, commit|reserve, nullptr);
}

auto VideoDevice::Refresh(std::stop_token stopee_v) -> void
try
{
	using namespace win32;
	using namespace std::chrono;
	using namespace std::chrono_literals;
	
	auto& display_v = m_Machine.GetDisplay();
	
	auto const interval_v = duration_cast<duration_type>(
		duration_cast<nanoseconds>(1s) / 60u);
	auto next_frame_v = filetime_clock::now();

	waitable_timer timer_v;	
	while(!stopee_v.stop_requested())
	{
		next_frame_v += interval_v;
		timer_v.set(next_frame_v);


		timer_v.wait();		
	}
}
catch (std::exception const& ex)
{}

VideoDeviceStateVga::VideoDeviceStateVga()	
{
	static_assert(std::is_trivially_copyable_v<VideoDeviceStateVga>);
	std::memset(this, 0, sizeof(*this));
	misc_output = 0x03u;	
}

auto VideoDeviceStateVga::IoPortWrite(std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t
{
	switch (port_v)
	{
	/***********************
	 *	RAM DAC
	 ***********************/
	case Port_DacPixelMask:
		ramdac.mask = data_v;
		return ERROR_SUCCESS;

	case Port_DacIndexWrite:
		ramdac.index = data_v * 3u;
		ramdac.latch = 0x3u;
		return ERROR_SUCCESS;

	case Port_DacIndexRead:
		ramdac.index = data_v * 3u;
		ramdac.latch = 0x0u;
		return ERROR_SUCCESS;

	case Port_DacDataWrite:
		ramdac.color[ramdac.index] = data_v & 0x3Fu;
		ramdac.index += 1u;
		while (ramdac.index >= 0x300u)
			ramdac.index -= 0x300u;
		return ERROR_SUCCESS;

	/***********************
	 *	CRT CONTROLLER
	 ***********************/
	case Port_MdaCrtIndex:
	case Port_VgaCrtIndex:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		crtctrl.index = data_v & 0x1Fu;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		return ERROR_SUCCESS;

	case Port_MdaCrtData:
	case Port_VgaCrtData:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		if (!(crtctrl.data[0x11u] & 0x80u)
			&& crtctrl.index < std::size(crtctrl.data))
			crtctrl.data[crtctrl.index] = data_v;
		crtctrl.index += 1u;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		return ERROR_SUCCESS;

	/***********************
	 *	SEQUENCER
	 ***********************/
	case Port_SequencerIndex:
		sequencer.index = data_v & 0x7u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		return ERROR_SUCCESS;

	case Port_SequencerData:
		sequencer.data[sequencer.index] = data_v;
		sequencer.index += 1u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		return ERROR_SUCCESS;

	/*********************************
	 *	GRAHPICS CONTROLLER
	 *********************************/
	case Port_GraphicsCtrlIndex:
		graphics.index = data_v & 0x0Fu;
		while (graphics.index >= std::size(graphics.data))
			graphics.index -= std::size(graphics.data);
		return ERROR_SUCCESS;
	case Port_GraphicsCtrlData:
		graphics.data[graphics.index] = data_v;
		graphics.index += 1u;
		while (graphics.index >= std::size(graphics.data))
			graphics.index -= std::size(graphics.data);
		return ERROR_SUCCESS;

	/***********************
	 *	ATTRIBUTE CONTROLLER
	 ***********************/
	case Port_Attribute0:
		if (!attrib.latch) {
			attrib.latch = !attrib.latch;
			attrib.index_and_pas = data_v&0x3Fu;			
		} else {
			attrib.latch = !attrib.latch;
			if (attrib.index < std::size(attrib.data))
				attrib.data[attrib.index] = data_v;			
		}
		return ERROR_SUCCESS;

	case Port_Attribute1:
		return ERROR_SUCCESS;

	/*********************************
	 *	MISC OUTPUT & FEATURE CONTROL
	 *********************************/
	case Port_MiscOutputWrite:
		misc_output = data_v;
		return ERROR_SUCCESS;

	case Port_MdaFeatureControl:
	case Port_VgaFeatureControl:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		feature_control = data_v;
		return ERROR_SUCCESS;

	default:
		break;
	}
	__debugbreak();
	return ERROR_SUCCESS;
}

auto VideoDeviceStateVga::IoPortFetch(std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>
{
	uint8_t tmp_v{ 0 };
	switch (port_v)
	{
	/***********************
	 *	RAM DAC
	 *******************/
	case Port_DacPixelMask:		
		return { ERROR_SUCCESS, ramdac.mask };

	case Port_DacDataRead:
		tmp_v = ramdac.color[ramdac.index];
		ramdac.index += 1u;
		while (ramdac.index >= 0x300u)
			ramdac.index -= 0x300u;
		return { ERROR_SUCCESS, tmp_v };

	case Port_DacStateRead:
		return { ERROR_SUCCESS, ramdac.latch };

	/***********************
	 *	CRT CONTROLLER
	 ***********************/
	case Port_MdaCrtIndex:
	case Port_VgaCrtIndex:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		return { ERROR_SUCCESS, crtctrl.index };

	case Port_VgaCrtData:
	case Port_MdaCrtData:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		tmp_v = crtctrl.data[crtctrl.index];
		crtctrl.index += 1u;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		return { ERROR_SUCCESS, tmp_v };

	case Port_MdaInputStatus:
	case Port_VgaInputStatus:
		if ((port_v < detail::VgaPort(0x3D0u)) == bool(misc_output & 0x1u)) break;
		attrib.latch = false;
		return { ERROR_SUCCESS, 0 };

	/***********************
	 *	SEQUENCER
	 ***********************/
	case Port_SequencerIndex:
		return { ERROR_SUCCESS, sequencer.index };

	case Port_SequencerData:
		tmp_v = sequencer.data[sequencer.index];
		sequencer.index += 1u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		return { ERROR_SUCCESS, tmp_v };

	/**************************
	 *	GRAPHICS CONTROLLER
	 **************************/
	case Port_GraphicsCtrlIndex:
		return { ERROR_SUCCESS, graphics.index };

	case Port_GraphicsCtrlData:
		tmp_v = graphics.data[graphics.index];
		graphics.index += 1u;
		while (graphics.index >= std::size(graphics.data))
			graphics.index -= std::size(graphics.data);
		return { ERROR_SUCCESS, tmp_v };

	/***********************
	 *	ATTRIBUTE CONTROLLER
	 ***********************/		
	case Port_Attribute0:
		return { ERROR_SUCCESS, attrib.index_and_pas };

	case Port_Attribute1:
		if (attrib.index < std::size(attrib.data))
			return { ERROR_SUCCESS, attrib.data[attrib.index] };
		return { ERROR_SUCCESS, 0x00u };

	/*********************************
	 *	MISC OUTPUT & FEATURE CONTROL
	 *********************************/
	case Port_MiscOutputRead:	
		return { ERROR_SUCCESS, misc_output };

	case Port_FeatureControlRead:
		return { ERROR_SUCCESS, feature_control };

	case Port_InputStatus:
		return { ERROR_SUCCESS, 0x00u };

	default:
		break;
	}

	__debugbreak();
	return { ERROR_SUCCESS, 0xffu };
}

auto VideoDeviceStateVga::HorizontalTotal() const -> uint16_t
{
	return (crtctrl.horizontal_total + 5u) * CharacterWidth();
}

auto VideoDeviceStateVga::VerticalTotal() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::HorizontalDisplayEnd() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::VerticalDisplayEnd() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::StartHorizontalRetrace() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::EndHorizontalRetrace() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::StartVerticalRetrace() const -> uint16_t
{
	return 0;
}

auto VideoDeviceStateVga::CharacterWidth() const -> uint8_t
{
	return sequencer.clocking_mode & 0x1u ? 8u : 9u;
}

auto VideoDeviceStateVga::CharacterHeight() const -> uint8_t
{
	return (crtctrl.maximum_scan_line & 0x1F) + 1u;
}

auto VideoDeviceStateVga::Log() const -> void
{
	using namespace std::string_literals;
	using utils::logger;

	std::string color_tbl;

	static constexpr const auto draw_rgb = [](auto&& r, auto&& g, auto&& b) {
		return std::format("\x1b[48;2;{};{};{}m  \x1b[0m", r, g, b);
	};
	static constexpr const auto draw_index_rgb = [](auto&& i, auto&& table) {
		return draw_rgb(table[3u * i + 0u]*4, table[3u * i + 1u]*4, table[3u * i + 2u]*4);
	};

	for (auto j = 0u; j < 0x10u; ++j)
	{
		color_tbl.append("\n  > ");
		for (auto i = 0u; i < 0x10u; ++i)
		{
			auto r = ramdac.color[3u * (j * 0x10u + i) + 0u];
			auto g = ramdac.color[3u * (j * 0x10u + i) + 1u];
			auto b = ramdac.color[3u * (j * 0x10u + i) + 2u];
			color_tbl.append(draw_rgb(r * 0x4, g * 0x4, b * 0x4));
		}
	}

	auto const resolution_w = uint32_t
		(	(crtctrl.end_horizontal_display + 1u)
		*	(sequencer.clocking_mode & 0x1u ? 8u : 9u));
	auto const resolution_h = uint32_t(crtctrl.vertical_total 
		+ ((crtctrl.crt_mode_control & 0x01u) * 0x100u)
		+ ((crtctrl.crt_mode_control & 0x20u) * 0x010u));

#define Fmt(X) std::format("  > " #X " = {}\n", X)
#define Fmt_(X, Y) std::format("  > " #X " = {} ({})\n", X, Y)
	logger::debug(logger::deflog, "video state : \n{}\n", 
		std::format("  > resolution : {} x {}\n", resolution_w, resolution_h)
		+Fmt(crtctrl.horizontal_total)
		+Fmt(crtctrl.end_horizontal_display)
		+Fmt(crtctrl.start_horizontal_blanking)
		+Fmt(crtctrl.end_horizontal_blanking)
		+Fmt(crtctrl.start_horizontal_retrace)
		+Fmt(crtctrl.end_horizontal_retrace)
		+Fmt(crtctrl.vertical_total)
		+Fmt(crtctrl.overflow)
		+Fmt(crtctrl.preset_row_scan)
		+Fmt(crtctrl.maximum_scan_line)
		+Fmt(crtctrl.cursor_start)
		+Fmt(crtctrl.cursor_end)
		+Fmt(crtctrl.start_address_high)
		+Fmt(crtctrl.start_address_low)
		+Fmt(crtctrl.cursor_location_high)
		+Fmt(crtctrl.cursor_location_low)
		+Fmt(crtctrl.vertical_retrace_start)
		+Fmt(crtctrl.vertical_retrace_end)
		+Fmt(crtctrl.vertical_display_end)
		+Fmt(crtctrl.offset)
		+Fmt(crtctrl.underline_location)
		+Fmt(crtctrl.start_vertical_blanking)
		+Fmt(crtctrl.end_vertical_blanking)
		+Fmt(crtctrl.crt_mode_control)
		+Fmt(crtctrl.line_compare)
		+Fmt(sequencer.reset)
		+Fmt(sequencer.clocking_mode)
		+Fmt(sequencer.map_mask)
		+Fmt(sequencer.character_map_select)
		+Fmt(sequencer.memory_mode)
		+Fmt(graphics.set_or_reset)
		+Fmt(graphics.enable_set_or_reset)
		+Fmt(graphics.color_compare)
		+Fmt(graphics.data_rotate)
		+Fmt(graphics.read_map_select)
		+Fmt(graphics.graphics_mode)
		+Fmt(graphics.miscellaneous)
		+Fmt(graphics.color_dont_care)
		+Fmt(graphics.bit_mask)
		+Fmt_(attrib.palette[0x0], draw_index_rgb(attrib.palette[0x0], ramdac.color))
		+Fmt_(attrib.palette[0x1], draw_index_rgb(attrib.palette[0x1], ramdac.color))
		+Fmt_(attrib.palette[0x2], draw_index_rgb(attrib.palette[0x2], ramdac.color))
		+Fmt_(attrib.palette[0x3], draw_index_rgb(attrib.palette[0x3], ramdac.color))
		+Fmt_(attrib.palette[0x4], draw_index_rgb(attrib.palette[0x4], ramdac.color))
		+Fmt_(attrib.palette[0x5], draw_index_rgb(attrib.palette[0x5], ramdac.color))
		+Fmt_(attrib.palette[0x6], draw_index_rgb(attrib.palette[0x6], ramdac.color))
		+Fmt_(attrib.palette[0x7], draw_index_rgb(attrib.palette[0x7], ramdac.color))
		+Fmt_(attrib.palette[0x8], draw_index_rgb(attrib.palette[0x8], ramdac.color))
		+Fmt_(attrib.palette[0x9], draw_index_rgb(attrib.palette[0x9], ramdac.color))
		+Fmt_(attrib.palette[0xA], draw_index_rgb(attrib.palette[0xA], ramdac.color))
		+Fmt_(attrib.palette[0xB], draw_index_rgb(attrib.palette[0xB], ramdac.color))
		+Fmt_(attrib.palette[0xC], draw_index_rgb(attrib.palette[0xC], ramdac.color))
		+Fmt_(attrib.palette[0xD], draw_index_rgb(attrib.palette[0xD], ramdac.color))
		+Fmt_(attrib.palette[0xE], draw_index_rgb(attrib.palette[0xE], ramdac.color))
		+Fmt_(attrib.palette[0xF], draw_index_rgb(attrib.palette[0xF], ramdac.color))		
		+Fmt(attrib.mode_control)
		+Fmt(attrib.overscan_color)
		+Fmt(attrib.color_plane_enable)
		+Fmt(attrib.horizontal_panning)
		+Fmt(attrib.color_select)
		+Fmt(ramdac.latch)
		+Fmt(ramdac.mask)		
	  +"  > ramdac.color => "s + color_tbl + "\n");

#undef Fmt
#undef Fmt_



}
