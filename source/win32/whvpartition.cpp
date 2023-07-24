#include <win32/whvpartition.hpp>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

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

auto WHvPartition::InitializeProcessor(std::uint32_t index_v) -> HRESULT
{
	auto result_v = ::WHvCreateVirtualProcessor(GetHandle(), index_v, 0u);
	if (S_OK != result_v) {
		return result_v;
	}
	win32::WHvProcessor processor_v{ *this, index_v };
	return processor_v.Reset();
}

WHvPartition::WHvPartition()
	: WHvPartition(nullptr)
{}

WHvPartition::WHvPartition(std::initializer_list<std::pair<WHV_PARTITION_PROPERTY_CODE, WHV_PARTITION_PROPERTY>> props_v)
	: WHvPartition(Create())
{
	WIN32_ERROR_ASSERT(InitializeProcessor(0u)); 
	for (auto const& [code_v, value_v] : props_v)
	{
		WIN32_ERROR_ASSERT(SetProperty(code_v, value_v));
		if (WHvPartitionPropertyCodeProcessorCount!=code_v)
			break;
		for (std::uint32_t index_v = 1u; 
			index_v < value_v.ProcessorCount; index_v += 1u) 
		{
			WIN32_ERROR_ASSERT(InitializeProcessor(index_v));
		}
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

auto WHvPartition::Processor(std::uint32_t apicid_v) const -> win32::WHvProcessor
{
  return WHvProcessor(*this, apicid_v);
}

WHvPartition::~WHvPartition() noexcept(false) {
	if (nullptr!=m_Handle) 
	{
		std::uint32_t cpucount_v{ 0u };
		WIN32_ERROR_ASSERT(GetProperty(WHvPartitionPropertyCodeProcessorCount, cpucount_v));
		for (std::uint32_t index_v = 0u; index_v < cpucount_v; index_v += 1u) {
			WIN32_ERROR_ASSERT(::WHvDeleteVirtualProcessor(m_Handle, index_v));
		}
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
