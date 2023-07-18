#include <win32/whvpartition.hpp>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utility>
using std::exchange;

using win32::WHvPartition;

auto WHvPartition::Create() -> WHvPartition
{
	WHV_PARTITION_HANDLE handle_v{ nullptr };
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&handle_v));
	return WHvPartition(handle_v);
}

auto WHvPartition::MapGpaRange(void* buffer_v, std::uint64_t physaddr_v, std::uint64_t length_v, WHV_MAP_GPA_RANGE_FLAGS flags_v) const -> HRESULT
{
	return ::WHvMapGpaRange(m_Handle, buffer_v, physaddr_v, length_v, flags_v);
}

auto WHvPartition::UnmapGpaRange(std::uint64_t physaddr_v, std::uint64_t length_v) const -> HRESULT
{
	return ::WHvUnmapGpaRange(m_Handle, physaddr_v, length_v);
}

WHvPartition::WHvPartition(WHV_PARTITION_HANDLE handle_v) noexcept
	: m_Handle{ handle_v }
{}

WHvPartition::WHvPartition()
	: WHvPartition(nullptr)
{}

WHvPartition::~WHvPartition() {
	if (nullptr!=m_Handle) {
		::WHvDeletePartition(m_Handle);
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

auto WHvPartition::SetProperty(WHV_PARTITION_PROPERTY_CODE property_v, void const* buffer_v, uint32_t size_v) const -> HRESULT
{
	return ::WHvSetPartitionProperty(m_Handle, property_v, buffer_v, size_v);
}

auto WHvPartition::GetProperty(WHV_PARTITION_PROPERTY_CODE property_v, void* buffer_v, uint32_t& size_v) const -> HRESULT
{
	return ::WHvGetPartitionProperty(m_Handle, property_v, buffer_v, size_v, &size_v);
}

