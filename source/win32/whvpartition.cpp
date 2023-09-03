#include <win32/whvpartition.hpp>
#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <utils/logger.hpp>

#include <utility>
using std::exchange;

using win32::WHvPartition;

WHvPartition::WHvPartition(WHV_PARTITION_HANDLE handle_v) noexcept
	: m_Handle{ handle_v }
{}

WHvPartition::~WHvPartition()
{
	using utils::logger;
	if (nullptr != m_Handle)
	{		
		auto [result_v, vcpucount_v] = GetProcessorCount(m_Handle);
		WIN32_ERROR_NOTIFY(result_v);
		if (result_v == S_OK)
		{
			for (std::uint32_t vcpuindex_v = 0u; 
				vcpuindex_v < vcpucount_v;
				vcpuindex_v += 1u)			
			{
				WIN32_ERROR_NOTIFY(::WHvDeleteVirtualProcessor(m_Handle, vcpuindex_v));
			}			
		}
		WIN32_ERROR_NOTIFY(::WHvDeletePartition(m_Handle));
		
	}
}

auto WHvPartition::swap(WHvPartition& with_v) noexcept -> void {
	std::swap(m_Handle, with_v.m_Handle);
}

WHvPartition::WHvPartition(WHvPartition&& from_v) noexcept 
	: m_Handle{ exchange(from_v.m_Handle, nullptr) }
{}

auto WHvPartition::operator=(WHvPartition&& from_v) noexcept -> WHvPartition& {	
	if (this != &from_v) {
		auto temp_v{ std::move(from_v) };
		temp_v.swap(*this);
	}
	return *this;
}

auto WHvPartition::SetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte const> value_v) -> std::int32_t
{
	return ::WHvSetPartitionProperty(handle_v, code_v, value_v.data(), value_v.size());
}

auto WHvPartition::GetProperty(WHV_PARTITION_HANDLE handle_v, WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte>& value_v) -> std::int32_t
{
	std::uint32_t size_o{ 0u };
	auto result_v = ::WHvGetPartitionProperty(handle_v, code_v, value_v.data(), value_v.size(), &size_o);
	value_v = value_v.first(size_o);
	return result_v;
}

auto WHvPartition::SetProperties(WHV_PARTITION_HANDLE handle_v, std::span<property_pair const> props_v) -> std::int32_t
{
	std::int32_t result_v{ ERROR_SUCCESS };
	for (auto const& [prop_k, prop_v] : props_v) {
		auto status_v = SetProperty(handle_v, prop_k, prop_v);
		if (ERROR_SUCCESS != status_v)
			result_v = status_v;		
	}
	return result_v;
}

auto WHvPartition::Create(std::uint32_t vcpucount_v, std::span<property_pair const> properties_v) -> WHV_PARTITION_HANDLE
{
	using utils::logger;
	WHV_PARTITION_HANDLE handle_v{ nullptr };
	std::uint32_t try_again_count_v{ 0u };
	Again:
	try
	{
		vcpucount_v = std::max(vcpucount_v, 1u);
		WIN32_ERROR_ASSERT(::WHvCreatePartition(&handle_v));
		//WIN32_ERROR_ASSERT(SetProcessorCount(handle_v, vcpucount_v));		
		WIN32_ERROR_ASSERT(SetProperties(handle_v, properties_v));
		WIN32_ERROR_ASSERT(::WHvSetupPartition(handle_v));
		for (std::uint32_t vcpuindex_v = 0u; vcpuindex_v < vcpucount_v; vcpuindex_v += 1u)
			WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(handle_v, vcpuindex_v, 0u));		
		return handle_v;
	}
	catch (win32::error const& ex)
	{	
		logger::error(logger::deflog, "{} failed: {}", __func__, ex.what());
		if (nullptr != handle_v) {
			WIN32_ERROR_NOTIFY(::WHvDeletePartition(handle_v));
		}
		try_again_count_v += 1u;
		if (try_again_count_v < 5u)
			goto Again;
		throw;
	}
	return nullptr;
}

auto WHvPartition::GetHandle() const -> WHV_PARTITION_HANDLE
{
	return m_Handle;
}

auto WHvPartition::Reset() const -> std::int32_t
{
	return ::WHvResetPartition(m_Handle);
}

auto WHvPartition::MapGpaRange(void* src_addr_v, std::uint64_t dst_addr_v, std::uint64_t size_v, core::Access access_v) const -> std::int32_t
{
	using enum core::Access;
	WHV_MAP_GPA_RANGE_FLAGS flags_v{ };
	if (kAccessFetch   & access_v) flags_v |= WHvMapGpaRangeFlagRead;
	if (kAccessWrite   & access_v) flags_v |= WHvMapGpaRangeFlagWrite;
	if (kAccessExecute & access_v) flags_v |= WHvMapGpaRangeFlagExecute;
	if (kTrackDirty    & access_v) flags_v |= WHvMapGpaRangeFlagTrackDirtyPages;

	assert(src_addr_v!=nullptr);

	if (dst_addr_v + size_v < dst_addr_v) {
		size_v = 0xFFFFFFFFFFFFFFFFull - dst_addr_v;
	}		
	return ::WHvMapGpaRange(m_Handle, src_addr_v, dst_addr_v, size_v, flags_v);
}

auto WHvPartition::UnmapGpaRange(std::uint64_t dst_addr_v, std::uint64_t size_v) const -> std::int32_t
{
	return ::WHvUnmapGpaRange(m_Handle, dst_addr_v, size_v);
}

auto WHvPartition::QueryGpaRangeDirtyBitmap(std::uint64_t address_v, std::uint64_t size_v, std::span<std::uint64_t> bitmap_v) const -> std::int32_t
{
	using namespace size_literals;
	assert(bitmap_v.size() * sizeof(std::uint64_t) * 8u >= (size_v + 1_pages - 1u) / 1_pages);
  return ::WHvQueryGpaRangeDirtyBitmap(m_Handle, address_v, size_v, bitmap_v.data(), bitmap_v.size() * sizeof(std::uint64_t));
}

auto WHvPartition::ClearGpaRangeDirtyBitmap(std::uint64_t address_v, std::uint64_t size_v) const -> std::int32_t
{
	return ::WHvQueryGpaRangeDirtyBitmap(m_Handle, address_v, size_v, nullptr, 0u);
}

auto WHvPartition::GetProcessorCount(WHV_PARTITION_HANDLE handle_v) -> std::tuple<std::int32_t, std::uint32_t> {
	std::uint32_t vcpucount_v{ 0u };
	auto result_v = GetProperty(handle_v, WHvPartitionPropertyCodeProcessorCount, vcpucount_v);
	return { result_v, vcpucount_v };
}

auto WHvPartition::SetProcessorCount(WHV_PARTITION_HANDLE handle_v, std::uint32_t vcpucount_v) -> std::int32_t {
	return SetProperty(handle_v, WHvPartitionPropertyCodeProcessorCount, vcpucount_v);	
}
