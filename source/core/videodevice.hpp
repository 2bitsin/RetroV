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

		enum class MemoryWindow: int {
			kNoMapping = -1,
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

		auto IoPortWrite(Processor const& vcpu_v, std::uint16_t port_v, std::uint8_t) -> std::int32_t;
		auto IoPortFetch(Processor const& vcpu_v, std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>;


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

	#pragma pack(push, 1)	
		struct GCreg_type
		{
			std::uint8_t index;
			// Set/Reset register index = 0x00
			union
			{
				struct
				{
					std::uint8_t value:4;
					std::uint8_t rsvd000:4;
				};
				std::uint8_t bits;
			} set_reset;

			// Enable set/reset register index = 0x01
			union
			{
				struct
				{
					std::uint8_t value:4;
					std::uint8_t rsvd100:4;
				};
				std::uint8_t bits;
			} enable_set_reset;

			// Color compare register index = 0x02
			union
			{
				struct
				{
					std::uint8_t value:4;
					std::uint8_t rsvd200:4;
				};
				std::uint8_t bits;
			} color_compare;

			// Data rotate register index = 0x03
			union
			{
				struct
				{
					std::uint8_t count:3;
					std::uint8_t operation:2;
					std::uint8_t rsvd300:3;
				};
				std::uint8_t bits;
			} data_rotate;

			// Read map select register index = 0x04
			union
			{
				struct
				{
					std::uint8_t value:2;
					std::uint8_t rsvd400:6;
				};
				std::uint8_t bits;
			} read_map_select;


			// Mode register index = 0x05
			union
			{
				struct
				{
					std::uint8_t write_mode:2;
					std::uint8_t rsvd500:1;
					std::uint8_t read_type:1;
					std::uint8_t odd_even:1;
					std::uint8_t shift_reg:1;
					std::uint8_t color_8bpp:1;
					std::uint8_t rsvd501:1;
				};
				std::uint8_t bits;
			} mode;

			// Miscellaneous register index = 0x06
			union
			{
				struct
				{
					std::uint8_t graphical_mode:1;
					std::uint8_t chain_odd_even:1;
					std::uint8_t memory_map:2;
				};
				std::uint8_t bits;
			} misc;

		} m_GCreg;
	#pragma pack(pop)
		
	};
}