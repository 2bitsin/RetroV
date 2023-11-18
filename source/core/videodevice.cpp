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
  if (m_RefreshTask.joinable())
    return;
  m_RefreshTask = std::jthread([this] (std::stop_token const& stopee_v) {
    while (stopee_v.stop_requested()) {
      RefreshTask(stopee_v);
    }
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

  std::unique_lock lock_v{ m_State_mut };
	if (is_write_v) {
		return m_State.IoPortWrite(port_v, data_v.as<std::uint8_t>());
  }
	auto const [status_v, value_v] = m_State.IoPortFetch(port_v);
  lock_v.unlock();
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
	auto const size_bytes_v = config_v.GetPropertyUint64("video.memory.size.kilobytes")*1_KiB;	
	m_VideoMemory = VirtualAlloc_s(size_bytes_v, read_write, commit|reserve, nullptr);
}

auto VideoDevice::RefreshTask(std::stop_token stopee_v) -> void 
{
  using namespace win32;
  using namespace std::chrono;
  using namespace std::chrono_literals;
  try
  { 
    waitable_timer timer_v;
    m_Hcounter = 0u;
    m_Vcounter = 0u;
    
    std::shared_lock lock_v{ m_State_mut };
    auto const state_v = m_State;
    lock_v.unlock();
    
    while (!stopee_v.stop_requested()) 
    {
      

    }  	
  }
  catch (std::exception const& ex)
  {
    
  }
}

