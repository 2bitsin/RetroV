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
	if (addr_v < 0xA0000u || addr_v >= 0xC0000u)
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
		if (crtctrl.index < std::size(crtctrl.data)) {
			if (!(crtctrl.data[0x11u] & 0x80u) || crtctrl.index > 0x07u) { 
				crtctrl.data[crtctrl.index] = data_v;
			}
		}
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
	//__debugbreak();
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

	//__debugbreak();
	return { ERROR_SUCCESS, 0xffu };
}


auto VideoDeviceStateVga::CharacterWidth() const -> uint8_t
{
	return sequencer.clocking_mode & 0x1u ? 8u : 9u;
}

auto VideoDeviceStateVga::CharacterHeight() const -> uint8_t
{	
	return (crtctrl.maximum_scan_line & 0x1F) + 1u;
}

auto VideoDeviceStateVga::ScanlineDouble() const -> bool
{
	return bool(crtctrl.maximum_scan_line & 0x80u);
}

auto VideoDeviceStateVga::ScanlineClockDivide() const -> bool
{
	return bool(crtctrl.crt_mode_control & 0x04u);
}

auto VideoDeviceStateVga::MemoryClockDivide() const -> bool
{
	return bool(crtctrl.crt_mode_control & 0x08u);
}

auto VideoDeviceStateVga::MasterClockDivide() const -> bool
{
	return bool(sequencer.character_map_select & 0x08u);
}

auto VideoDeviceStateVga::MasterClockRate() const -> uint64_t
{
	switch ((misc_output & 0xCu) >> 2u)
	{
	default  : 
	case 0x0 : return 25175000ull;
	case 0x1 : return 28322000ull;
	case 0x2 : return 31500000ull;
	case 0x3 : return 40000000ull;
	}	
}

auto VideoDeviceStateVga::HorizontalTotal() const -> uint16_t
{
	return CharacterWidth() * (crtctrl.horizontal_total + 5u);
}

auto VideoDeviceStateVga::HorizontalDisplayEnd() const -> uint16_t
{
	return CharacterWidth() * (crtctrl.horizontal_display_end + 1u);
}

auto VideoDeviceStateVga::HorizontalRetraceStart() const -> uint16_t
{
	return CharacterWidth() * crtctrl.horizontal_retrace_start;
}

auto VideoDeviceStateVga::HorizontalRetraceEnd() const -> uint16_t
{
	auto const lsb_v = crtctrl.horizontal_retrace_end & 0x1Fu;
	auto const counter_v = lsb_v + (HorizontalRetraceStart() & ~0x1Fu);
	if (counter_v > HorizontalTotal())
		return counter_v;
	return lsb_v;
}

auto VideoDeviceStateVga::HorizontalBlankingStart() const -> uint16_t
{	
	return CharacterWidth() * crtctrl.horizontal_blanking_start;
}

auto VideoDeviceStateVga::HorizontalBlankingEnd() const -> uint16_t
{	
	auto const lsb_v = (
		 ((crtctrl.horizontal_blanking_end & 0x1Fu) >> 0u) +
		 ((crtctrl.horizontal_retrace_end  & 0x80u) >> 2u));
	auto const counter_v = lsb_v + (HorizontalBlankingStart() & ~0x1Fu);
	if (counter_v > HorizontalTotal())
		return counter_v;
	return lsb_v;
}

auto VideoDeviceStateVga::VerticalTotal() const -> uint16_t
{
	return crtctrl.vertical_total
		+ 0x100u * (crtctrl.overflow & 0x01u)
		+ 0x010u * (crtctrl.overflow & 0x20u)
		;
}

auto VideoDeviceStateVga::VerticalDisplayEnd() const -> uint16_t
{
	return crtctrl.vertical_blanking_end
		+ 0x80u*(crtctrl.overflow & 0x02u)
		+ 0x08u*(crtctrl.overflow & 0x40u)
		;	
}

auto VideoDeviceStateVga::VerticalRetraceStart() const -> uint16_t
{
	return crtctrl.vertical_retrace_start 
		+ 0x40u*(crtctrl.overflow & 0x04u)
		+ 0x04u*(crtctrl.overflow & 0x80u)
		;
}

auto VideoDeviceStateVga::VerticalRetraceEnd() const -> uint16_t
{
	auto const lsb_v = crtctrl.vertical_retrace_end & 0x0Fu;
	auto counter_v = lsb_v + (VerticalRetraceStart() & ~0x0Fu);
	if (counter_v > VerticalTotal())
		return counter_v;
	return lsb_v;
}

auto VideoDeviceStateVga::VerticalBlankingStart() const -> uint16_t
{
	return crtctrl.vertical_blanking_start 
		+ (crtctrl.maximum_scan_line & 0x20u) * 0x10u
		+ (crtctrl.overflow & 0x08u) * 0x20u
		; 
}

auto VideoDeviceStateVga::VerticalBlankingEnd() const -> uint16_t
{
	auto const lsb_v = crtctrl.vertical_blanking_end & 0x7Fu;
	auto counter_v = lsb_v + (VerticalBlankingStart() & ~0x7Fu);
	if (counter_v > VerticalTotal())
		return counter_v;
	return lsb_v;
}

auto VideoDeviceStateVga::DisplayEnableSkew() const -> uint8_t
{
	return (crtctrl.horizontal_blanking_end >> 5u) & 3u;
}

auto VideoDeviceStateVga::HorizontalRetraceSkew() const -> uint8_t
{
	return (crtctrl.horizontal_retrace_end >> 5u) & 3u;
}

auto VideoDeviceStateVga::CursorSkew() const -> uint8_t
{
	return (crtctrl.cursor_end >> 5u) & 3u;
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
		return draw_rgb(
			table[3u * i + 0u]*4, 
			table[3u * i + 1u]*4,
			table[3u * i + 2u]*4
		);
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

#define Fmt(X)     std::format("  > {:.<35} = {}\n"      , #X, X)
#define Fmt_(X, Y) std::format("  > {:.<35} = {} ({})\n" , #X, X, Y)
#define FmtH(X)    std::format("  > {:.<35} = 0x{:02X}\n", #X, X)
	logger::debug(logger::deflog, "video state : \n{}\n", 
		std::string()

		+Fmt(HorizontalTotal())
		+Fmt(HorizontalDisplayEnd())
		+Fmt(HorizontalBlankingStart())
		+Fmt(HorizontalRetraceStart())
		+Fmt(HorizontalRetraceEnd())
		+Fmt(HorizontalBlankingEnd())

		+Fmt(VerticalTotal())
		+Fmt(VerticalDisplayEnd())
		+Fmt(VerticalBlankingStart())
		+Fmt(VerticalRetraceStart())
		+Fmt(VerticalRetraceEnd())
		+Fmt(VerticalBlankingEnd())

		+Fmt(DisplayEnableSkew())
		+Fmt(HorizontalRetraceSkew())
		+Fmt(CursorSkew())

		+Fmt(ScanlineDouble())
		+Fmt(ScanlineClockDivide())
		+Fmt(MemoryClockDivide())
		+Fmt(MasterClockRate()*1e-6)

		+Fmt(CharacterWidth())
		+Fmt(CharacterHeight())

		+FmtH(crtctrl.horizontal_total)
		+FmtH(crtctrl.horizontal_display_end)
		+FmtH(crtctrl.horizontal_blanking_start)
		+FmtH(crtctrl.horizontal_blanking_end)
		+FmtH(crtctrl.horizontal_retrace_start)
		+FmtH(crtctrl.horizontal_retrace_end)
		+FmtH(crtctrl.vertical_total)
		+FmtH(crtctrl.overflow)
		+FmtH(crtctrl.preset_row_scan)
		+FmtH(crtctrl.maximum_scan_line)
		+FmtH(crtctrl.cursor_start)
		+FmtH(crtctrl.cursor_end)
		+FmtH(crtctrl.start_address_high)
		+FmtH(crtctrl.start_address_low)
		+FmtH(crtctrl.cursor_location_high)
		+FmtH(crtctrl.cursor_location_low)
		+FmtH(crtctrl.vertical_retrace_start)
		+FmtH(crtctrl.vertical_retrace_end)
		+FmtH(crtctrl.vertical_display_end)
		+FmtH(crtctrl.offset)
		+FmtH(crtctrl.underline_location)
		+FmtH(crtctrl.vertical_blanking_start)
		+FmtH(crtctrl.vertical_blanking_end)
		+FmtH(crtctrl.crt_mode_control)
		+FmtH(crtctrl.line_compare)
		+FmtH(sequencer.reset)
		+FmtH(sequencer.clocking_mode)
		+FmtH(sequencer.map_mask)
		+FmtH(sequencer.character_map_select)
		+FmtH(sequencer.memory_mode)
		+FmtH(graphics.set_or_reset)
		+FmtH(graphics.enable_set_or_reset)
		+FmtH(graphics.color_compare)
		+FmtH(graphics.data_rotate)
		+FmtH(graphics.read_map_select)
		+FmtH(graphics.graphics_mode)
		+FmtH(graphics.miscellaneous)
		+FmtH(graphics.color_dont_care)
		+FmtH(graphics.bit_mask)
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
		+FmtH(attrib.mode_control)
		+FmtH(attrib.overscan_color)
		+FmtH(attrib.color_plane_enable)
		+FmtH(attrib.horizontal_panning)
		+FmtH(attrib.color_select)
		+FmtH(ramdac.latch)
		+FmtH(ramdac.mask)		
	  +"  > ramdac.color => "s + color_tbl + "\n");

#undef Fmt
#undef Fmt_
#undef FmtH


}
