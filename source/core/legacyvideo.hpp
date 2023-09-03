#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <deque>
#include <list>

#include <core/display.hpp>
#include <core/mapgparange.hpp>

#include <win32/error.hpp>
#include <win32/memory.hpp>
#include <win32/mappedfile.hpp>

#include <utils/region.hpp>
#include <utils/smart_span.hpp>
#include <utils/span.hpp>

namespace core
{
	struct Machine;
	struct Processor;

	struct LegacyVideo
	{
		using duration_type = std::chrono::microseconds;

		LegacyVideo(Machine& machine_v);
		~LegacyVideo();
		
		auto Initialize() -> void;

		auto Start() -> void;
		auto Stop() -> void;
		auto Restart() -> void;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;

		auto Refresh(Display::surface_tmp& surface_v) -> duration_type;

	protected:
		auto RefreshThread(std::stop_token stoppee_v) -> void;

	private:
		Machine& m_Machine;

		std::stop_source m_Stopper;
		std::future<void> m_Refresh;

		std::uint16_t m_Height;
		std::uint16_t m_Width;

		std::list<MapGpaRange> m_MappedRanges;
		std::list<win32::MappedFile> m_MappedROMs;

		win32::unique_span<std::byte> m_VideoMemory;
		win32::unique_span<std::byte> m_BackBuffer;	

		static inline constexpr const utils::region64_type s_MemoryWindow [] = {
			{ utils::from_range, 0x000B0000u, 0x00008000u },
			{ utils::from_range, 0x000B8000u, 0x00008000u },
			{ utils::from_range, 0x000A0000u, 0x00010000u }
		};

		static inline constexpr const utils::region64_type s_ROMWindow []  = {
			{ utils::from_range, 0x000C8000u, 0x000D0000u }
		};

		

	};
}