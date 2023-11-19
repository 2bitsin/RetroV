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

#include <core/videodevice/vgaregisters.hpp>
#include <core/videodevice/vgarenderer.hpp>
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

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> std::int32_t;
		auto Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;

	protected:

		auto ConfigureROM(core::Configuration const&) -> void;
		auto ConfigureMemory(core::Configuration const&) -> void;

		auto RefreshTask(std::stop_token stopee_v) -> void;

    auto FrameTime() const -> std::uint32_t;


	protected:


	private:

    #pragma pack(push, 1)
    struct port_write_item 
    { 
      std::uint32_t time;
      std::uint32_t data; 
      std::uint16_t addr;
      std::uint16_t size;
    };
    #pragma pack(pop)

    using port_write_queue = utils::coqueue<port_write_item>;

		Machine& m_Machine;
		std::optional<RomImage> m_BiosRom;

		buffer_type m_VideoMemory;

		VGARegisters m_VgaRegisters;
    VGARenderer m_VgaRenderer;

    port_write_queue m_PortWriteQueue;
    std::jthread m_RefreshTask;
  };
}