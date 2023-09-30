#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvregisters.hpp>

#include <algorithm>

using win32::WHvProcessor;

WHvProcessor::WHvProcessor(WHvPartition& partition_v, std::uint32_t vcpuindex_v)
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
	static constexpr WHV_REGISTER_VALUE const values_s [] = {
		{.Segment = {.Base = 0xF0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } },
		{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } },
		{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } },
		{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } },
		{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0093u } },
		{.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		{.Table = {.Limit = 0x03FFu, .Base = 0x00000000u  } },
		{.Table = {.Limit = 0x0000u, .Base = 0x00000000u  } },
		{.Reg64 = 0x0000'0000'0000'0002u },
		{.Reg64 = 0x0000'0000'0000'FFF0u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'6000'0010u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u },
		{.Reg64 = 0x0000'0000'0000'0000u }
	};

	static constexpr WHV_REGISTER_NAME const names_s[] = {
		WHvX64RegisterCs,
		WHvX64RegisterEs,
		WHvX64RegisterDs,
		WHvX64RegisterFs,
		WHvX64RegisterGs,
		WHvX64RegisterSs,
		WHvX64RegisterIdtr,
		WHvX64RegisterGdtr,
		WHvX64RegisterRflags,
		WHvX64RegisterRip,
		WHvX64RegisterRbx,
		WHvX64RegisterRcx,
		WHvX64RegisterRdx,
		WHvX64RegisterRsi,
		WHvX64RegisterRdi,
		WHvX64RegisterRbp,
		WHvX64RegisterRsp,
		WHvX64RegisterR8,
		WHvX64RegisterR9,
		WHvX64RegisterR10,
		WHvX64RegisterR11,
		WHvX64RegisterR12,
		WHvX64RegisterR13,
		WHvX64RegisterR14,
		WHvX64RegisterR15,
		WHvX64RegisterCr0,
		WHvX64RegisterCr2,
		WHvX64RegisterCr3,
		WHvX64RegisterCr4
	};

	return SetRegisters(names_s, values_s);
}

auto WHvProcessor::RunToExit(WHV_RUN_VP_EXIT_CONTEXT& exit_v) const -> std::int32_t
{
	return ::WHvRunVirtualProcessor(m_Partition.GetHandle(), m_VcpuIndex, &exit_v, sizeof(exit_v));
}

auto WHvProcessor::GetIndex() const -> std::uint32_t
{
	return m_VcpuIndex;
}

auto WHvProcessor::RunToExit() const -> std::tuple<std::int32_t, WHV_RUN_VP_EXIT_CONTEXT>
{
	WHV_RUN_VP_EXIT_CONTEXT exit_v{};
	auto const result_v = RunToExit(exit_v);
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

auto WHvProcessor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 16u> buffer_v, WHV_CACHE_TYPE cache_v) const -> std::int32_t
{
	auto const cc_v = WHV_ACCESS_GPA_CONTROLS{ .CacheType = cache_v };
	if (!is_write_v) {
		return ::WHvReadGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), buffer_v.size());
	} else {
		return ::WHvWriteGpaRange(m_Partition.GetHandle(), m_VcpuIndex, physaddr_v, cc_v, buffer_v.data(), buffer_v.size());
	}
}

auto WHvProcessor::RequestInterrupt(WHV_INTERRUPT_CONTROL irq_v) -> std::int32_t {
  return ::WHvRequestInterrupt(m_Partition.GetHandle(), &irq_v, sizeof(irq_v));
}

auto WHvProcessor::GetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::vector<std::byte>& buffer_v) const -> std::int32_t
{
	std::int32_t result_v{ 0 };
	do {
		std::uint32_t osize_v{ 0u };
		result_v = ::WHvGetVirtualProcessorState(m_Partition.GetHandle(), m_VcpuIndex, 
			type_v, buffer_v.data(), buffer_v.size(), &osize_v);
		buffer_v.resize(osize_v);			
	} while(result_v == WHV_E_INSUFFICIENT_BUFFER);
	return result_v;
}

auto WHvProcessor::SetState(WHV_VIRTUAL_PROCESSOR_STATE_TYPE type_v, std::span<std::byte const> buffer_v) const -> std::int32_t
{
	return ::WHvSetVirtualProcessorState(m_Partition.GetHandle(), m_VcpuIndex, type_v, buffer_v.data(), buffer_v.size());
}

