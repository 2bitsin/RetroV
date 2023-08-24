#include <win32/winhvpx.hpp>

#include <core/machine.hpp>
#include <core/legacyvideo.hpp>

#include <utils/literals.hpp>

using core::LegacyVideo;

LegacyVideo::LegacyVideo(Machine& machine_v)
	: m_Machine{ machine_v }
	, m_B0000toB7FFF{ std::nullopt }
	, m_B8000toBFFFF{ std::nullopt }
	, m_A0000toAFFFF{ std::nullopt }
	, m_Height{ 0 }
	, m_Width{ 0 }
{}

auto LegacyVideo::Initialize() -> std::int32_t
{
	using namespace size_literals;

	m_B8000toBFFFF.emplace(m_Machine.GetPartition(), 0xB8000u, 0x8000u, kAccessDevice|kTrackDirty);		
	m_ScratchPages = std::make_unique<Page[]>(32_KiB/1_pages);
	assert(((std::uint64_t)m_ScratchPages.get() & 0xFFFu) == 0u);
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

auto LegacyVideo::Render() -> std::tuple<utils::buffer2d<std::uint32_t>, std::chrono::microseconds>
{
	using namespace size_literals;
	using namespace std::chrono_literals;

	utils::buffer2d<std::uint32_t> frame_buffer_v { m_Width, m_Height };
	m_Machine.SuspendAllProcessors();

	std::span target_v { &m_ScratchPages[0]._[0], 32_KiB };	
	std::span source_v { m_B8000toBFFFF->Data(), m_B8000toBFFFF->Size() } ;


  return { std::move(frame_buffer_v), 16666us };
}
