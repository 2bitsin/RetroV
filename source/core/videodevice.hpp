#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <list>

#include <core/videodevice/bscharacter.hpp>
#include <core/videodevice/bsgraphical.hpp>
#include <core/videodevice/bscommon.hpp>
#include <core/videodevice/ramdac.hpp>

#include <core/configuration.hpp>
#include <core/mapgparange.hpp>
#include <core/romimage.hpp>
#include <core/display.hpp>

#include <utils/limited_span.hpp>
#include <utils/smart_span.hpp>
#include <utils/region.hpp>
#include <utils/span.hpp>

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
		using duration_type = videodevice::duration_type;
		using buffer_type = videodevice::buffer_type;
		using region_type = utils::region64_type;

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
		auto IoPortWrite(Processor const& vcpu_v, std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t;
		auto IoPortFetch(Processor const& vcpu_v, std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>;
		auto ConfigureROM(core::Configuration const&) -> void;
		auto ConfigureMemory(core::Configuration const&) -> void;
		
		auto GetMemoryRegion(region_type const& region_v, uint32_t flags_v) const -> std::tuple<std::int32_t, std::size_t, std::span<std::byte>>;
		auto MemorySize() const -> std::size_t;
		
	protected:
		auto Hypercall_SetMode(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;
		auto Hypercall_MemoryMap(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;		
		auto Hypercall_SetView(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;

	private:	
		Machine& m_Machine;		
		using graphical_mode_type = videodevice::BsGraphical;
		using character_mode_type = videodevice::BsCharacter;
		using device_mode = std::variant<character_mode_type, graphical_mode_type>;
		
		device_mode m_VideoMode;
		std::optional<RomImage> m_BiosRom;
		std::list<MapGpaRange> m_MemoryMap;
		buffer_type m_VideoMemory [2u];		
		RamDAC m_RamDAC;
	};
}