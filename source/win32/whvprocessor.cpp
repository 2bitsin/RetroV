#include <win32/whvprocessor.hpp>
#include <algorithm>

using win32::WHvProcessor;

WHvProcessor::WHvProcessor(win32::WHvPartition& partition_v, std::uint32_t vcpuindex_v)
	: m_Partition{ partition_v }
	, m_VcpuIndex{ vcpuindex_v }
{
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, 0u));
}

WHvProcessor::~WHvProcessor() noexcept(false)
{
	WIN32_ERROR_ASSERT(::WHvDeleteVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex));
}

auto WHvProcessor::TranslateGva(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const
	-> std::tuple<HRESULT, WHV_TRANSLATE_GVA_RESULT, std::uint64_t> 
{
	WHV_TRANSLATE_GVA_RESULT code_v{};
	std::uint64_t physaddr_v{};
	auto result_v = ::WHvTranslateGva(m_Partition.GetHandle(), m_VcpuIndex, virtaddr_v, flags_v, &code_v, &physaddr_v);
	return { result_v, code_v, physaddr_v };
}

auto WHvProcessor::Run() const -> std::tuple<HRESULT, WHV_RUN_VP_EXIT_CONTEXT>
{
	WHV_RUN_VP_EXIT_CONTEXT exit_v{};
	auto result_v = ::WHvRunVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, &exit_v, sizeof(exit_v));
	return { result_v, exit_v };
}

auto WHvProcessor::Cancel() const -> HRESULT
{
	return WHvCancelRunVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, 0);
}

auto WHvProcessor::GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> HRESULT
{
	assert (rnames_v.size() == values_v.size());
	if (rnames_v.size() != values_v.size())
		return E_INVALIDARG;
	return WHvGetVirtualProcessorRegisters(m_Partition.GetHandle(), m_VcpuIndex, rnames_v.data(), rnames_v.size(), values_v.data());
}

auto WHvProcessor::SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> HRESULT
{
	assert (rnames_v.size() == values_v.size());
	if (rnames_v.size() != values_v.size())
		return E_INVALIDARG;
	return WHvSetVirtualProcessorRegisters(m_Partition.GetHandle(), m_VcpuIndex, rnames_v.data(), rnames_v.size(), values_v.data());
}

auto WHvProcessor::MemFetchSome(std::uint64_t physaddr_v, std::span<std::byte> buffer_v, WHV_CACHE_TYPE cache_v) const -> std::tuple<HRESULT, std::size_t>
{
	auto const cc_v = WHV_ACCESS_GPA_CONTROLS{ .CacheType = cache_v };
	auto size_v = std::min(kMaxMemoryAccessSize, buffer_v.size());
	return { ::WHvReadGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), size_v), size_v };
}

auto WHvProcessor::MemWriteSome(std::uint64_t physaddr_v, std::span<std::byte const> buffer_v, WHV_CACHE_TYPE cache_v) const -> std::tuple<HRESULT, std::size_t>
{
	auto const cc_v = WHV_ACCESS_GPA_CONTROLS{ .CacheType = cache_v };
	auto size_v = std::min(kMaxMemoryAccessSize, buffer_v.size());
	return { ::WHvWriteGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), size_v), size_v };
}
