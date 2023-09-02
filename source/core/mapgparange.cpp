#include <core/mapgparange.hpp>

using core::MapGpaRange;

MapGpaRange::MapGpaRange(win32::WHvPartition& partition_v, utils::region_64_t region_v, Access flags_v, std::span<std::byte> view_v)
	: m_Partition	{ partition_v }
	, m_Region		{ region_v		}
	, m_Flags			{ flags_v			}
	, m_View			{ view_v			}
	, m_Enabled		{ false				}
{	
	Enable();
}

MapGpaRange::~MapGpaRange()
{
	Disable();
}

auto MapGpaRange::Enable() -> void {	
	if (!m_Enabled) {
		auto const size_v = std::min(m_Region.size(), m_View.size());
		WIN32_ERROR_ASSERT(m_Partition.MapGpaRange(m_View.data(), m_Region.base(), size_v, m_Flags));
		m_Enabled = true;	
	}
}


auto MapGpaRange::Disable() -> void {
	if (m_Enabled) {		
		auto const size_v = std::min(m_Region.size(), m_View.size());
		WIN32_ERROR_ASSERT(m_Partition.UnmapGpaRange(m_Region.base(), size_v));
		m_Enabled = false;
	}
}
