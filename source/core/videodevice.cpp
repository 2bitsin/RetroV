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

VideoDevice::VideoDevice(core::Machine& machine_v)
	: m_Machine{ machine_v }
	, m_CharGen{ *this }
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
	m_RefreshThread = std::jthread{ 
		utils::lambda(this, &VideoDevice::Refresh)
	};
}

auto VideoDevice::Stop() -> void
{
	if (m_RefreshThread.joinable()) 
	{
		m_RefreshThread.request_stop();
		m_RefreshThread.join();
	}
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
		return IoPortWrite(vcpu_v, port_v, data_v.as<std::uint8_t>());		
	auto const [status_v, value_v] = IoPortFetch(vcpu_v, port_v);
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
	return ERROR_SUCCESS;
}

auto VideoDevice::Hypercall_SetVideoMode(uint16_t horiz_v, uint16_t vert_v, uint16_t mode_v, uint16_t flags_v) -> std::int32_t
{
	return ERROR_SUCCESS;
}

auto VideoDevice::IoPortWrite(Processor const& vcpu_v, std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t
{
	switch (port_v)
	{
	case 0x016u: // 0x3C6
	case 0x017u: // 0x3C7
	case 0x018u: // 0x3C8
	case 0x019u: // 0x3C9
		return m_RamDAC.IoPortWrite(port_v - 0x016u, data_v);

	case 0x024u: // 0x3D4
	case 0x025u: // 0x3D5
		return m_CrtCtrl.IoPortWrite(port_v - 0x024u, data_v);

	default: 
		break;
	}
	return ERROR_ACCESS_DENIED;
}

auto VideoDevice::IoPortFetch(Processor const& vcpu_v, std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>
{
	switch (port_v)
	{
	case 0x016u: // 0x3C6
	case 0x017u: // 0x3C7
	case 0x018u: // 0x3C8
	case 0x019u: // 0x3C9
		return m_RamDAC.IoPortFetch(port_v - 0x016u);

	case 0x024u: // 0x3D4
	case 0x025u: // 0x3D5
		return m_CrtCtrl.IoPortFetch(port_v - 0x024u);

	default:
		break;
	}
	return { ERROR_ACCESS_DENIED, 0 };
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

auto VideoDevice::GetMemoryRegion(region_type const& region_v, uint32_t flags_v) const
	-> std::tuple<std::int32_t, std::size_t, std::span<std::byte>>
{
	using namespace win32;
	assert(0u == (region_v.base() & 0xFFFu));
	assert(0u == (region_v.size() & 0xFFFu));
	auto source_s = m_VideoMemory[0].subspan(region_v.base(), region_v.size());
	auto target_s = m_VideoMemory[1].subspan(region_v.base(), region_v.size());
	auto [status_v, copied_v] =	CopyDirtyPages(target_s, source_s);
	if (ERROR_SUCCESS!=status_v) return { status_v, 0, {} };
	return { ERROR_SUCCESS, copied_v, target_s };
}

auto VideoDevice::MemorySize() const -> std::size_t
{
	return m_MemoryMap.size();
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

