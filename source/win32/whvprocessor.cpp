#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvregisters.hpp>

#include <algorithm>

using win32::WHvProcessor;

WHvProcessor::WHvProcessor(WHvPartition const& partition_v, std::uint32_t vcpuindex_v)
	: m_Partition{ partition_v }
	, m_VcpuIndex{ vcpuindex_v }
{}

auto WHvProcessor::TranslateGva(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v) const
	-> std::tuple<std::int32_t, WHV_TRANSLATE_GVA_RESULT_CODE, std::uint64_t>
{
	WHV_TRANSLATE_GVA_RESULT code_v{};
	std::uint64_t physaddr_v{};
	auto result_v = ::WHvTranslateGva(m_Partition.GetHandle(), m_VcpuIndex, virtaddr_v, flags_v, &code_v, &physaddr_v);
	return { result_v, code_v.ResultCode, physaddr_v };
}

auto WHvProcessor::Reset() const -> std::int32_t
{
	return GetInitialProcessorState().ApplyTo(*this);
}

auto WHvProcessor::Run(WHV_RUN_VP_EXIT_CONTEXT& exit_v) const -> std::int32_t
{
	return ::WHvRunVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, &exit_v, sizeof(exit_v));
}

auto WHvProcessor::Run() const -> std::tuple<std::int32_t, WHV_RUN_VP_EXIT_CONTEXT>
{
	WHV_RUN_VP_EXIT_CONTEXT exit_v{};
	auto const result_v = Run(exit_v);
	return { result_v, exit_v };
}

auto WHvProcessor::Cancel() const -> std::int32_t
{
	return WHvCancelRunVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, 0);
}

auto WHvProcessor::GetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t
{
	assert (rnames_v.size() == values_v.size());
	if (rnames_v.size() != values_v.size())
		return E_INVALIDARG;
	return WHvGetVirtualProcessorRegisters(m_Partition.GetHandle(), m_VcpuIndex, rnames_v.data(), rnames_v.size(), values_v.data());
}

auto WHvProcessor::SetRegisters(std::span<WHV_REGISTER_NAME const> rnames_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t
{
	assert (rnames_v.size() == values_v.size());
	if (rnames_v.size() != values_v.size())
		return E_INVALIDARG;
	return WHvSetVirtualProcessorRegisters(m_Partition.GetHandle(), m_VcpuIndex, rnames_v.data(), rnames_v.size(), values_v.data());
}

auto WHvProcessor::SetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE value_v) const -> std::int32_t
{
	return SetRegisters({ &rname_v, 1u }, { &value_v, 1u });
}

auto WHvProcessor::GetRegister(WHV_REGISTER_NAME rname_v, WHV_REGISTER_VALUE& value_v) const -> std::int32_t
{
	return GetRegisters({ &rname_v, 1u }, { &value_v, 1u });
}

auto WHvProcessor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> buffer_v, WHV_CACHE_TYPE cache_v) const -> std::int32_t
{
	auto const cc_v = WHV_ACCESS_GPA_CONTROLS{ .CacheType = cache_v };
	
	if (!is_write_v) {
		return ::WHvReadGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), buffer_v.size());
	} else {
		return ::WHvWriteGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), buffer_v.size());
	}
}
