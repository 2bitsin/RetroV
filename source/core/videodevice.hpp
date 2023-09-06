#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <list>

#include <core/display.hpp>
#include <core/mapgparange.hpp>
#include <core/configuration.hpp>

#include <utils/limited_span.hpp>
#include <utils/region.hpp>
#include <utils/smart_span.hpp>
#include <utils/span.hpp>

#include <win32/error.hpp>
#include <win32/memory.hpp>
#include <win32/mappedfile.hpp>

namespace core
{
	struct Machine;	
	struct Processor;	

	struct VideoDevice
	{
		using duration_type = std::chrono::microseconds;

		enum class MemoryWindow: std::uint8_t {
			kGraphical = 0,
			kTextLower = 1,
			kTextUpper = 2
		};

		VideoDevice(Machine& machine_v);
		~VideoDevice();

		auto Initialize(Configuration const& config_v) -> void;
		auto Start() -> void;
		auto Stop() -> void;
		auto Restart() -> void;
		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;
		auto Refresh(Display::surface_tmp& surface_v) -> duration_type;

	protected:
		auto ConfigureBiosROM(Configuration const& config_v) -> void;
		auto ConfigureMemory(Configuration const& config_v) -> void;

		auto SetLegacyMapping(MemoryWindow target_v, std::size_t offset_v) -> void;

		auto RefreshThread(std::stop_token stoppee_v) -> void;

	private:
		using buffer_type = win32::unique_span<std::byte>;

		Machine& m_Machine;		

		std::stop_source m_Stopper;
		std::future<void> m_Refresh;

		std::uint16_t m_Height;
		std::uint16_t m_Width;

		std::array<buffer_type, 2u> m_VideoMemory;
		std::list<MapGpaRange> m_MappedMemory;

		std::optional<win32::MappedFile> m_MappedRomFile;
		std::optional<MapGpaRange> m_MappedRomRange;
		
	};
}