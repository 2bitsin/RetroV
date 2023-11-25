#include <core/videodevice.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>

#include <win32/waitabletimer.hpp>
#include <win32/async.hpp>

#include <utils/logger.hpp>
#include <utils/algorithm.hpp>
#include <utils/validate.hpp>
#include <utils/literals.hpp>
#include <utils/surface.hpp>
#include <utils/lambda.hpp>
#include <utils/paths.hpp>

#include <bios/vmcall.h>

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
	ConfigureMemory(config_v);
}

auto VideoDevice::Start() -> void
{
  if (m_RefreshTask.joinable())
    return;
  m_RefreshTask = std::jthread([this] (std::stop_token const& stopee_v) {
    while (stopee_v.stop_requested()) {
      RefreshTask(stopee_v);
    }
    win32::drain_apc_queue();
  });
}

auto VideoDevice::Stop() -> void
{
  if (m_RefreshTask.joinable())
  {
    m_RefreshTask.request_stop();
    m_RefreshTask.join();
  }
}

auto VideoDevice::Restart() -> void
{
	Stop();
	Start();
}

auto VideoDevice::IoPortAccess(Processor const& vcpu_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t
{	
  auto const time_v = RelativeFrameTime();
  if (is_write_v) {
    m_IoWriteQueue.push(time_v, port_v, data_v.size(), 
      data_v.as<uint32_t>(), vcpu_v.GetIndex());
    return ERROR_SUCCESS;
  }

  auto const result_v = WaitRenderUntil(time_v);
  if (result_v != ERROR_SUCCESS)
    return result_v;
  auto const value_v = IoPortFetch(vcpu_v, port_v, data_v.size());
  return ERROR_SUCCESS;
}

auto VideoDevice::MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> int32_t
{
	if (addr_v < 0xA0000u || addr_v >= 0xC0000u)
		__debugbreak();
	return ERROR_SUCCESS;
}

auto VideoDevice::Hypercall(Processor const& vcpu_v, HypercallContext const& context_v) -> int32_t
{	
	using namespace win32::regs;

	auto const& hypercall_v = context_v.Hypercall;
	auto const& vpcontext_v = context_v.VpContext;

	switch (context_v.Function) {
	case HYPERCALL_VIDEO_BEGIN_UPDATE:
    Stop();
		return ERROR_SUCCESS;
	case HYPERCALL_VIDEO_END_UPDATE:
		Start();
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
	auto const size_bytes_v = std::max<size_t>(config_v.GetPropertyUint64("video.memory.size.kilobytes"), 256u)*1_KiB;	
	m_VideoMemory[0] = VirtualAlloc_s(size_bytes_v, read_write, commit | reserve, nullptr);
  m_VideoMemory[1] = VirtualAlloc_s(size_bytes_v, read_write, commit | reserve, nullptr);
  m_DirtyVramMask  = VirtualAlloc_s(size_bytes_v, read_write, commit | reserve, nullptr);
}

auto VideoDevice::RefreshTask(std::stop_token stopee_v) -> void 
{
  using namespace std::chrono_literals;
  using namespace std::chrono;
  using namespace win32;

}

auto VideoDevice::SetClockFrequency(uint32_t value_v) -> void
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  
  
}

auto VideoDevice::ResetFrameTimer() -> void
{

}

auto VideoDevice::RelativeFrameTime() const -> uint32_t
{
  return 0;
}

auto VideoDevice::WaitRenderUntil(uint32_t time_v) -> int32_t
{
  return int32_t();
}

auto VideoDevice::IoPortFetch(Processor const& vcpu_v, uint16_t port_v, size_t size_v) -> uint32_t
{
  return uint32_t();
}

auto VideoDevice::HostFetch(VGARegisters const& state_v, uint32_t addr_v, uint8_t size_v) -> uint32_t
{ 
  uint32_t data_v{ 0u };
  switch (size_v) 
  {
  default: throw std::invalid_argument("Invalid size");    
  case 4u: size_v -= 1u; data_v = (data_v << 8u) | (HostFetchByte(state_v, addr_v + size_v) & 0xFF); [[fallthrough]];
  case 3u: size_v -= 1u; data_v = (data_v << 8u) | (HostFetchByte(state_v, addr_v + size_v) & 0xFF); [[fallthrough]];
  case 2u: size_v -= 1u; data_v = (data_v << 8u) | (HostFetchByte(state_v, addr_v + size_v) & 0xFF); [[fallthrough]];
  case 1u: size_v -= 1u; data_v = (data_v << 8u) | (HostFetchByte(state_v, addr_v + size_v) & 0xFF); break; 
  }    
  return data_v;
}

auto VideoDevice::HostFetchByte(VGARegisters const& state_v, uint32_t addr_v) -> uint8_t
{
  using enum VGARegisters::ValueIndex;

  if (state_v.GetValue<ReadModeSelect>()) {
    __debugbreak();
    throw std::runtime_error("Read mode 1 not implemented.");
  }

  
}

auto VideoDevice::HostWrite(bool is_ahead_v, VGARegisters const& state_v, uint32_t addr_v, uint32_t data_v, uint8_t size_v) -> void
{
  switch (size_v)
  {
  default: throw std::invalid_argument("Invalid size");
  case 4u: HostWriteByte(is_ahead_v, state_v, addr_v, data_v & 0xFFu); addr_v += 1u; data_v >>= 8u; [[fallthrough]];
  case 3u: HostWriteByte(is_ahead_v, state_v, addr_v, data_v & 0xFFu); addr_v += 1u; data_v >>= 8u; [[fallthrough]];
  case 2u: HostWriteByte(is_ahead_v, state_v, addr_v, data_v & 0xFFu); addr_v += 1u; data_v >>= 8u; [[fallthrough]];
  case 1u: HostWriteByte(is_ahead_v, state_v, addr_v, data_v & 0xFFu); addr_v += 1u; data_v >>= 8u; break;
  }
}

auto VideoDevice::HostWriteByte(bool is_ahead_v, VGARegisters const& state_v, uint32_t addr_v, uint8_t value_v) -> void
{
  using 
  auto const read_mode_v = state_v.GetValue<
}

