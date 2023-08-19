#include <core/localapic.hpp>
#include <core/machine.hpp>
#include <core/processor.hpp>
#include <win32/winhvpx.hpp>

using core::LocalApic;

LocalApic::LocalApic(Machine& machine_v, std::uint32_t vcpu_index_v)
	: m_Machine{ machine_v }
	, m_Processor{ machine_v.GetProcessor(vcpu_index_v) }
{}

auto LocalApic::Initialize() -> std::int32_t
{
	return m_Processor.GetState(WHvVirtualProcessorStateTypeInterruptControllerState2, m_InitState);
}

auto LocalApic::XApicWrite(std::uint16_t register_v, std::uint32_t value_v) const -> std::int32_t
{	
	if (register_v >= 0x100u)
		return E_INVALIDARG;
	auto [result_v, base_v, enable_v] = GetBase();
	result_v = m_Processor.MemoryWrite(base_v + register_v*0x10u, value_v);
	if (FAILED(result_v))
		return result_v;	
	return S_OK;
}

auto LocalApic::XApicFetch(std::uint16_t register_v) const -> std::tuple<std::int32_t, std::uint32_t> 
{
	if (register_v >= 0x100u)
		return { E_INVALIDARG, 0 };
	auto [result_v, base_v, enable_v] = GetBase();
	if (FAILED(result_v))
		return { result_v, 0 };
	WHV_REGISTER_VALUE value_v{ };
	result_v = m_Processor.MemoryFetch(base_v + register_v*0x10u, value_v.Reg32);
	if (FAILED(result_v))
		return { result_v, 0 };
	return { S_OK, value_v.Reg32 };
}

auto LocalApic::GetBase() const -> std::tuple<std::int32_t, std::uint64_t, bool>
{
	WHV_REGISTER_VALUE base_r{ };
	auto result_v = m_Processor.GetRegister(WHvX64RegisterApicBase, base_r);
	if (FAILED(result_v))
		return { result_v, 0, false };
	auto const base_v = base_r.Reg64 & ~0xFFFull;
	auto const enable_v = !!(base_r.Reg64 & 0x800u);
	return { S_OK, base_v, enable_v };
}

auto LocalApic::SetBase(std::uint64_t base_v, bool enable_v) const -> std::int32_t
{
	return m_Processor.SetRegister(WHvX64RegisterApicBase, { 
		.Reg64 = (base_v & ~0xFFFull) | (enable_v ? 0x800u : 0u) 
	});
}

auto LocalApic::SignalEOI() const -> std::int32_t
{
	auto [result_v, base_v, enable_v] = GetBase();
	if (FAILED(result_v))
		return result_v;
	return m_Processor.MemoryWrite(base_v + EOIR, 0u);
}

auto LocalApic::RequestIRQ(WHV_INTERRUPT_TRIGGER_MODE mode_v, WHV_INTERRUPT_TYPE type_v, std::uint8_t vector_v, WHV_INTERRUPT_DESTINATION_MODE dest_v) const -> std::int32_t
{
	WHV_INTERRUPT_CONTROL irq_v{ };
	irq_v.TriggerMode = mode_v,
	irq_v.Type = type_v;
	irq_v.DestinationMode = dest_v;
	irq_v.Destination = m_Processor.GetIndex();
	irq_v.Vector = vector_v;
	auto& partition_v = m_Machine.GetPartition();
	auto const result_v = ::WHvRequestInterrupt(partition_v.GetHandle(), &irq_v, sizeof(irq_v));
	if (FAILED(result_v))
		return result_v;
	m_Processor.Unsuspend();
	return S_OK;
}

