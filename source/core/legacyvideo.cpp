#include <win32/winhvpx.hpp>

#include <core/machine.hpp>
#include <core/legacyvideo.hpp>

using core::LegacyVideo;

LegacyVideo::LegacyVideo(Machine& machine_v)
	: m_Machine{ machine_v }
{}

auto LegacyVideo::Initialize() -> std::int32_t
{
	m_VideoRAM.emplace_back(
		m_Machine.GetPartition(), 
		0xB8000u, 0x8000u, 
		kAccessDevice|kTrackDirty);
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
