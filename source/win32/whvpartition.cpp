#include <win32/whvpartition.hpp>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utility>
using std::exchange;

using win32::WHvPartition;

auto WHvPartition::Create() -> WHV_PARTITION_HANDLE
{
	WHV_PARTITION_HANDLE handle_v{ nullptr };
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&handle_v));
	return handle_v;
}

WHvPartition::WHvPartition(WHV_PARTITION_HANDLE handle_v) noexcept
	: m_Handle{ handle_v }
{}

WHvPartition::WHvPartition()
	: WHvPartition(Create())
{}

WHvPartition::WHvPartition(std::initializer_list<std::pair<WHV_PARTITION_PROPERTY_CODE, WHV_PARTITION_PROPERTY>> props_v)
	: WHvPartition(Create())
{
	for (auto const& [code_v, value_v] : props_v) 
	{
		WIN32_ERROR_ASSERT(SetProperty(code_v, value_v));
	}
	WIN32_ERROR_ASSERT(Setup());
}

auto WHvPartition::MapGpaRange(void* buffer_v, std::uint64_t physaddr_v, std::uint64_t length_v, core::Access flags_v) const -> HRESULT
{
	WHV_MAP_GPA_RANGE_FLAGS whv_flags_v{ };
	using enum core::Access;

	if (flags_v & kAccessFetch   ) whv_flags_v |= WHvMapGpaRangeFlagRead;	
	if (flags_v & kAccessWrite   ) whv_flags_v |= WHvMapGpaRangeFlagWrite;
	if (flags_v & kAccessExecute ) whv_flags_v |= WHvMapGpaRangeFlagExecute;		
	
	return ::WHvMapGpaRange(m_Handle, buffer_v, physaddr_v, length_v, whv_flags_v);
}

auto WHvPartition::UnmapGpaRange(std::uint64_t physaddr_v, std::uint64_t length_v) const -> HRESULT
{
	return ::WHvUnmapGpaRange(m_Handle, physaddr_v, length_v);
}

WHvPartition::~WHvPartition() noexcept(false) {
	if (nullptr!=m_Handle) {
		WIN32_ERROR_ASSERT(::WHvDeletePartition(m_Handle));
	}
}

WHvPartition::WHvPartition(WHvPartition&& other_v) noexcept
	: m_Handle{ exchange(other_v.m_Handle, nullptr) }
{}

auto WHvPartition::operator=(WHvPartition&& other_v) noexcept -> WHvPartition&
{
	if (this != &other_v) {
		auto temp_v{ std::move(other_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WHvPartition::swap(WHvPartition& other_v) noexcept -> void 
{
	std::swap(m_Handle, other_v.m_Handle);
}

auto WHvPartition::GetHandle() const noexcept -> WHV_PARTITION_HANDLE
{
	return m_Handle;
}

auto WHvPartition::Setup() const -> HRESULT
{
	return ::WHvSetupPartition(m_Handle);
}

auto WHvPartition::Reset() const -> HRESULT
{
	return ::WHvResetPartition(m_Handle);
}

auto WHvPartition::SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, std::span<std::byte const> value_v) const -> HRESULT
{
	return ::WHvSetPartitionProperty(m_Handle, property_v, value_v.data(), (std::uint32_t)value_v.size());
}

auto WHvPartition::GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, std::span<std::byte>& value_v) const -> HRESULT
{
	std::uint32_t size_v{ 0 };
	auto result_v = ::WHvGetPartitionProperty(m_Handle, property_v, value_v.data(), (std::uint32_t)value_v.size(), &size_v);
	value_v = value_v.first(size_v);
	return result_v;
}

