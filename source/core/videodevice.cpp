#include <core/videodevice.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

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
using core::VgaState;

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

		if (data_v.size() >= 1u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 0u, data_v.subspan(1u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() >= 2u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 1u, data_v.subspan(2u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() >= 3u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 2u, data_v.subspan(3u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		if (data_v.size() >= 4u) 
			status_v = IoPortAccess(vcpu_v, is_write_v, port_v + 3u, data_v.subspan(4u, 1u)); 
		if (status_v != ERROR_SUCCESS) 
			return status_v;

		return ERROR_SUCCESS;
	}

	if (is_write_v) 
		return m_State.IoPortWrite(port_v, data_v.as<std::uint8_t>());		
	auto const [status_v, value_v] = m_State.IoPortFetch(port_v);
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

auto VgaState::IoPortWrite(std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t
{
	switch (port_v)
	{
		/***********************
		 *	RAM DAC
		 *******************/
	case Port_DacIndexWrite:
		ramdac.index = data_v * 3u;
		ramdac.latch = 0x3u;
		break;
	case Port_DacIndexRead:
		ramdac.index = data_v * 3u;
		ramdac.latch = 0x0u;
		break;
	case Port_DacDataWrite:
		ramdac.color[ramdac.index] = data_v & 0x3Fu;
		ramdac.index += 1u;
		while (ramdac.index >= 0x300u)
			ramdac.index -= 0x300u;
		break;
		/***********************
		 *	CRTC
		 *******************/
	case Port_VgaCrtIndex:
		crtctrl.index = data_v & 0x1Fu;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		break;
	case Port_VgaCrtData:
		if (!(crtctrl.data[0x11u] & 0x80u)
			&& crtctrl.index < std::size(crtctrl.data))
			crtctrl.data[crtctrl.index] = data_v;
		crtctrl.index += 1u;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		break;
		/***********************
		 *	SEQ
		 *******************/
	case Port_SequencerIndex:
		sequencer.index = data_v & 0x7u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		break;
	case Port_SequencerData:
		sequencer.data[sequencer.index] = data_v;
		sequencer.index += 1u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		break;
	default:
		break;
	}
	return ERROR_SUCCESS;
}

auto VgaState::IoPortFetch(std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>
{
	uint8_t tmp_v{ 0 };
	switch (port_v)
	{
		/***********************
		 *	RAM DAC
		 *******************/
	case Port_DacDataRead:
		tmp_v = ramdac.color[ramdac.index];
		ramdac.index += 1u;
		while (ramdac.index >= 0x300u)
			ramdac.index -= 0x300u;
		return { ERROR_SUCCESS, tmp_v };
	case Port_DacStateRead:
		return { ERROR_SUCCESS, ramdac.latch };
		/***********************
		 *	CRTC
		 *******************/
	case Port_VgaCrtIndex:
		return { ERROR_SUCCESS, crtctrl.index };
	case Port_VgaCrtData:
		tmp_v = crtctrl.data[crtctrl.index];
		crtctrl.index += 1u;
		while (crtctrl.index >= std::size(crtctrl.data))
			crtctrl.index -= std::size(crtctrl.data);
		return { ERROR_SUCCESS, tmp_v };
		/***********************
		 *	SEQ
		 *******************/
	case Port_SequencerIndex:
		return { ERROR_SUCCESS, sequencer.index };
	case Port_SequencerData:
		tmp_v = sequencer.data[sequencer.index];
		sequencer.index += 1u;
		while (sequencer.index >= std::size(sequencer.data))
			sequencer.index -= std::size(sequencer.data);
		return { ERROR_SUCCESS, tmp_v };
	default:
		break;
	}
	return { ERROR_SUCCESS, 0 };
}
