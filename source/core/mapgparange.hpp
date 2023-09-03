#pragma once

#include <core/accessflags.hpp>

#include <win32/error.hpp>
#include <win32/whvpartition.hpp>

#include <utils/region.hpp>

#include <cstddef>
#include <cstdint>
#include <span>



namespace core
{
	
	struct MapGpaRange 
	{		
		MapGpaRange(win32::WHvPartition& partition_v, utils::region64_type region_v, Access flags_v, std::span<std::byte> view_v);
		~MapGpaRange();
		auto Disable() -> void;
		auto Enable() -> void;
	private:
		win32::WHvPartition& m_Partition;
		utils::region64_type m_Region;
		Access m_Flags;
		std::span<std::byte> m_View;		
		bool m_Enabled;
	};

}

