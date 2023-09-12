#include <core/mapgparange.hpp>
#include <win32/memory.hpp>

using core::MapGpaRange;

MapGpaRange::MapGpaRange(win32::WHvPartition& partition_v, utils::region64_type region_v, Access flags_v, std::span<std::byte> view_v)
	: m_Partition	{ &partition_v }
	, m_Region		{ region_v		 }
	, m_Flags			{ flags_v			 }
	, m_View			{ view_v			 }
	, m_Enabled		{ false				 }
{	
	Enable();
}

MapGpaRange::~MapGpaRange()
{
	Disable();
}

MapGpaRange::MapGpaRange(MapGpaRange&& from_v) noexcept
	: m_Partition	{ std::exchange(from_v.m_Partition, nullptr) }
	, m_Region		{ std::exchange(from_v.m_Region		, {})      }
	, m_Flags			{ std::exchange(from_v.m_Flags		, {})      }
	, m_View			{ std::exchange(from_v.m_View			, {})      }
	, m_Enabled		{ std::exchange(from_v.m_Enabled	, false)   }
{

}

auto MapGpaRange::operator=(MapGpaRange&& from_v) noexcept -> MapGpaRange&
{
	if (this != &from_v) {
		MapGpaRange tmp_v(std::move(from_v));
		tmp_v.swap(*this);
	}
	return *this;
}

auto MapGpaRange::swap(MapGpaRange& other_v) noexcept -> void
{
	std::swap(m_Partition, other_v.m_Partition);
	std::swap(m_Region, other_v.m_Region);
	std::swap(m_Flags, other_v.m_Flags);
	std::swap(m_View, other_v.m_View);
}

auto MapGpaRange::Enable() -> void {	
	if (!m_Partition || m_View.empty()) 
		throw std::logic_error("Invalid MapGpaRange");
	if (!m_Enabled) {
		auto const size_v = std::min(m_Region.size(), m_View.size());
	#ifndef NDEBUG
		s_log.MapGpaRange(m_View.data(), m_Region.base(), size_v, m_Flags);
	#endif
		WIN32_ERROR_ASSERT(m_Partition->MapGpaRange(m_View.data(), m_Region.base(), size_v, m_Flags));
		m_Enabled = true;	
	}
}

auto MapGpaRange::Remap(region_type target_v, Access flags_v) -> void
{
	Disable();
	m_Region = target_v;
	m_Flags = flags_v;
	Enable();
}


auto MapGpaRange::Disable() -> void {
	if (!m_Partition || m_View.empty())
		throw std::logic_error("Invalid MapGpaRange");
	if (m_Enabled) {
		auto const size_v = std::min(m_Region.size(), m_View.size());
	#ifndef NDEBUG
		s_log.UnmapGpaRange(m_Region.base(), size_v);
	#endif
		WIN32_ERROR_ASSERT(m_Partition->UnmapGpaRange(m_Region.base(), size_v));
		m_Enabled = false;
	}
}
