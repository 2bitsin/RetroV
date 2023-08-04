#include <core/processor.hpp>
#include <core/machine.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>

using core::Processor;

Processor::Processor(Machine& machine_v, std::uint32_t vcpuindex_v)
	: WHvProcessor{ machine_v.Partition(), vcpuindex_v }
	, m_Machine{ machine_v }	
{}

Processor::~Processor()
{}

auto Processor::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t
{
	return m_Machine.IoPortAccess(is_write_v, port_v, data_v);
}

auto Processor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t
{	
	return WHvProcessor::MemoryAccess(is_write_v, physaddr_v, data_v);
}

auto Processor::GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t
{
	return WHvProcessor::GetRegisters(names_v, values_v);
}

auto Processor::SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t
{
	return WHvProcessor::SetRegisters(names_v, values_v);
}

auto Processor::TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_o, std::uint64_t& addr_o) const -> std::int32_t
{
	auto const [status_v, code_v, addr_v] = WHvProcessor::TranslateGva(virtaddr_v, flags_v);
	addr_o = addr_v; code_o = code_v; return status_v;
}
