#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <shared_mutex>
#include <mutex>
#include <list>


#include <core/configuration.hpp>
#include <core/mapgparange.hpp>
#include <core/romimage.hpp>
#include <core/display.hpp>

#include <core/videodevice/vgaioconsts.hpp>
#include <core/videodevice/vgaregisters.hpp>
#include <core/videodevice/vgarenderer.hpp>
#include <core/videodevice/videoclock.hpp>
#include <core/videodevice/iowritequeue.hpp>


#include <utils/limited_span.hpp>
#include <utils/smart_span.hpp>
#include <utils/region.hpp>
#include <utils/span.hpp>
#include <utils/coqueue.hpp>

#include <win32/chrono.hpp>
#include <win32/mappedfile.hpp>
#include <win32/memory.hpp>
#include <win32/error.hpp>

namespace core
{
	struct Machine;
	struct Processor;
	struct HypercallContext;

  

	struct VideoDevice
	{
    using io_write_queue = videodevice::io_write_queue;
		using duration_type = win32::filetime_clock::duration;
		using buffer_type = win32::unique_span<std::byte>;
		using region_type = utils::region64_type;

    using VGARegisters = videodevice::VGARegisters;
    using VGARenderer = videodevice::VGARenderer;


		VideoDevice(Machine& machine_v);
		~VideoDevice();

		auto Initialize(Configuration const& config_v) -> void;
		auto Start() -> void;
		auto Stop() -> void;
		auto Restart() -> void;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> int32_t;
		auto Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> int32_t;

	protected:

		auto ConfigureROM(core::Configuration const&) -> void;
		auto ConfigureMemory(core::Configuration const&) -> void;

		auto RefreshTask(std::stop_token stopee_v) -> void;

    auto SetClockFrequency(uint32_t value_v) -> void;
    auto ResetFrameTimer() -> void;
    auto RelativeFrameTime() const -> uint32_t;
    auto WaitRenderUntil(uint32_t time_v) -> int32_t;
    auto IoPortFetch(Processor const& vcpu_v, uint16_t port_v, size_t size_v) -> uint32_t;

    auto HostFetch(VGARegisters const& state_v, uint32_t addr_v, uint8_t size_v) -> uint32_t;
    auto HostFetchByte(VGARegisters const& state_v, uint32_t addr_v) -> uint8_t;
    auto HostWrite(bool is_ahead_v, VGARegisters const& state_v, uint32_t addr_v, uint32_t value_v, uint8_t size_v) -> void;
    auto HostWriteByte(bool is_ahead_v, VGARegisters const& state_v, uint32_t addr_v, uint8_t value_v) -> void;

    auto VRamView(size_t buff_v, size_t plane_v) -> std::span<uint8_t>;
    
	private:

		Machine& m_Machine;
		std::optional<RomImage> m_BiosRom;

		buffer_type m_VideoMemory [2u];
    buffer_type m_DirtyVramMask;

		VGARegisters m_VgaRegisters;
    VGARenderer m_VgaRenderer;

    io_write_queue m_IoWriteQueue;
    std::jthread m_RefreshTask;

    
  };
}