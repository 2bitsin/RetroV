#include <core/virtualprocessor.hpp>
#include <core/virtualmachine.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvpartition.hpp>

using core::VirtualProcessor;

VirtualProcessor::VirtualProcessor(VirtualMachine& machine_v, std::uint32_t vcpuindex_v)
	: m_Machine{ machine_v }
	, m_VcpuIndex{ vcpuindex_v }
{
}

VirtualProcessor::~VirtualProcessor()
{}

auto VirtualProcessor::IoPortAccess(bool is_write_v, std::uint16_t port_v, std::uint8_t size_v, std::span<std::byte, 4u> data_v) -> std::int32_t
{
	return m_Machine.IoPortAccess(is_write_v, port_v, size_v, data_v);
}

auto VirtualProcessor::MemoryAccess(bool is_write_v, std::uint64_t physaddr_v, std::uint8_t size_v, std::span<std::byte, 8u> data_v) -> std::int32_t
{
	using namespace win32;
	std::size_t _{ 0 };
	WHvProcessor processor_v{ m_Machine.Partition(), m_VcpuIndex };	
	std::int32_t result_v{ 0 };
	processor_v.MemoryAccess(is_write_v, physaddr_v, size_v, data_v);
	return result_v;
}

auto VirtualProcessor::GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) -> std::int32_t
{
	using namespace win32;
	WHvProcessor processor_v{ m_Machine.Partition(), m_VcpuIndex };
	return processor_v.GetRegisters(names_v, values_v);
}

auto VirtualProcessor::SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) -> std::int32_t
{
	using namespace win32;
	WHvProcessor processor_v{ m_Machine.Partition(), m_VcpuIndex };
	return processor_v.SetRegisters(names_v, values_v);
}

auto VirtualProcessor::TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_v, std::uint64_t& physaddr_v) -> std::int32_t
{
	using namespace win32;
	WHvProcessor processor_v{ m_Machine.Partition(), m_VcpuIndex };
	HRESULT result_v{ 0 };  
	std::tie(result_v, code_v, physaddr_v) = processor_v.TranslateGva(virtaddr_v, flags_v);
	return result_v;
}
