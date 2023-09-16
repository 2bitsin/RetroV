#include <core/videodevice.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <bios/com/hypercall.hpp>

#include <utils/algorithm.hpp>
#include <utils/validate.hpp>
#include <utils/literals.hpp>
#include <utils/surface.hpp>
#include <utils/lambda.hpp>
#include <utils/paths.hpp>

#include <win32/whvcapabilities.hpp>

#include <algorithm>
#include <chrono>
#include <ranges>

using core::VideoDevice;

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
}

auto VideoDevice::Start() -> void
{}

auto VideoDevice::Stop() -> void
{}

auto VideoDevice::Restart() -> void
{}


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

auto VideoDevice::Hypercall_SetMode(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t {
	auto const& vpcontext_v = hypercall_v.VpContext;
	auto const& hccontext_v = hypercall_v.Hypercall;

	auto const [type_v, flags_v] = 
		utils::integral_split<uint16_t>((uint32_t)hccontext_v.Rbx);
	auto const [horizontal_v, vertical_v] =
		utils::integral_split<uint16_t>((uint32_t)hccontext_v.Rcx);

	return ERROR_SUCCESS;
}

auto VideoDevice::Hypercall_MemoryMap(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t
{
	auto const& vpcontext_v = hypercall_v.VpContext;
	auto const& hccontext_v = hypercall_v.Hypercall;

	using namespace size_literals;
	using region_type = utils::region64_type;

	auto source_v = std::min(hccontext_v.Rsi&0xFFFFFFFFu, m_VideoMemory.size());
	auto length_v = std::min(hccontext_v.Rcx&0xFFFFFFFFu, m_VideoMemory.size());
	auto target_v = hccontext_v.Rdi&0xFFFFFFFFu;
	auto flags_v = hccontext_v.Rbx&0xFFFFFFFFu;

	if (flags_v&1u) m_MemoryMap.clear();
	m_MemoryMap.emplace_back(m_Machine.GetPartition(), 
		region_type{ target_v, length_v }, kAccessDevice,
		m_VideoMemory.subspan(source_v, length_v)
	);

	return ERROR_SUCCESS;
}

auto VideoDevice::Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t
{
	using namespace core::hypercall;
	switch (hypercall_v.Minor)
	{
	case HYPERCALL_VIDEO_SET_MODE:
		return Hypercall_SetMode(vcpu_v, hypercall_v);
	case HYPERCALL_VIDEO_MEMORY_MAP:	
	case HYPERCALL_VIDEO_SET_VIEW:
		return ERROR_SUCCESS;
	}
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
	default: break;
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
	m_VideoMemory = VirtualAlloc_s(size_bytes_v, read_write, commit|reserve|write_watch, nullptr);
}
