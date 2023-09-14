#pragma once

#include <core/accessflags.hpp>
#include <core/eventlog.hpp>

#include <win32/error.hpp>
#include <win32/mappedfile.hpp>
#include <win32/whvpartition.hpp>

#include <utils/region.hpp>

#include <cstddef>
#include <cstdint>
#include <span>

namespace core
{	
	struct MapGpaRange 
	{		
		using region_type = utils::region64_type;
		using partition_type = win32::WHvPartition;

		inline MapGpaRange(partition_type& partition_v, region_type region_v, Access flags_v, auto const& other_v)
			: MapGpaRange(partition_v, region_v, flags_v, std::span<std::byte>(other_v))
		{}

		MapGpaRange(win32::WHvPartition& partition_v, utils::region64_type region_v, Access flags_v, std::span<std::byte> view_v);
		~MapGpaRange();

		MapGpaRange(MapGpaRange const&) = delete;
		auto operator=(MapGpaRange const&) -> MapGpaRange& = delete;

		MapGpaRange(MapGpaRange&&) noexcept ;
		auto operator=(MapGpaRange&&) noexcept -> MapGpaRange&;

		auto swap (MapGpaRange& other_v) noexcept -> void;

		auto Disable() -> void;
		auto Enable() -> void;
		auto Remap(region_type target_v, Access flags_v) -> void;
	private:
		win32::WHvPartition* m_Partition;
		utils::region64_type m_Region;
		Access m_Flags;
		std::span<std::byte> m_View;		
		bool m_Enabled;
		std::optional<std::filesystem::path> m_Path;
		static inline const EventLog s_log{ "Memory" };
	};

}

