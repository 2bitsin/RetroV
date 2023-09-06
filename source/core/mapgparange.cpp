#include <core/mapgparange.hpp>
#include <win32/memory.hpp>

using core::MapGpaRange;

MapGpaRange::MapGpaRange(win32::WHvPartition& partition_v, utils::region64_type region_v, Access flags_v, std::span<std::byte> view_v)
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
	#ifndef NDEBUG
		std::visit([&]<typename T>(T const& what_v) {
			if constexpr (std::is_same_v<T, std::filesystem::path>) {				
				s_log.MapGpaRangeFromFile(m_View.data(), m_Region.base(), size_v, what_v, 0, size_v); 
			} else {
				s_log.MapGpaRange(m_View.data(), m_Region.base(), size_v, m_Flags);
			}
		}, win32::GetMappedFileName(m_View.data()));

	#endif
		WIN32_ERROR_ASSERT(m_Partition.MapGpaRange(m_View.data(), m_Region.base(), size_v, m_Flags));
		m_Enabled = true;	
	}
}


auto MapGpaRange::Disable() -> void {
	if (m_Enabled) {		
		auto const size_v = std::min(m_Region.size(), m_View.size());
	#ifndef NDEBUG
		s_log.UnmapGpaRange(m_Region.base(), size_v);
	#endif
		WIN32_ERROR_ASSERT(m_Partition.UnmapGpaRange(m_Region.base(), size_v));
		m_Enabled = false;
	}
}
