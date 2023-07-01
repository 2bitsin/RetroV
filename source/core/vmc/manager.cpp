#include <core/vmc/manager.hpp>
#include <core/hypervisor.hpp>

using core::vmc::Manager;

Manager::Manager(core::Hypervisor& hypervisor_v)
	: m_Hypervisor(&hypervisor_v)
	, m_LastID(1u)
{
	m_Callbacks.reserve(kLastCallNumber+1u);
}

Manager::~Manager()
{}


Manager::Manager(Manager&& prev_v) noexcept
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_LastID(std::exchange(prev_v.m_LastID, 0u))
	, m_Callbacks(std::move(prev_v.m_Callbacks))
{}

auto Manager::operator=(Manager&& prev_v) noexcept -> Manager&
{
	if (this != &prev_v) {
		auto temp_v{ std::move(prev_v) };
		temp_v.Swap(*this);
	}
	return *this;
}

auto Manager::Swap(Manager& other_v) noexcept -> void
{
	std::swap(m_Hypervisor, other_v.m_Hypervisor);
	std::swap(m_LastID, other_v.m_LastID);
	std::swap(m_Callbacks, other_v.m_Callbacks);
}

auto Manager::RegisterCallback(std::uint16_t callno_v, std::function<vmcall_callback> callback_v) -> std::uint32_t
{
	if (callno_v >= m_Callbacks.size())
		m_Callbacks.resize(callno_v+1u);
	m_Callbacks[callno_v].emplace_back(m_LastID, callback_v);
	return std::exchange(m_LastID, m_LastID+1u);
}

auto Manager::UnregisterCallback(std::uint16_t callno_v, std::uint32_t target_v) -> void
{
	if (callno_v >= m_Callbacks.size())
		return;
	auto &callbacks_v = m_Callbacks[callno_v];
	for (auto curr_v = callbacks_v.begin(); curr_v != callbacks_v.end(); ++curr_v) {
		if (auto const& [curr_slot_v, _] = *curr_v; curr_slot_v == target_v) {
			callbacks_v.erase(curr_v);
			break; }}
}

auto Manager::DispatchCallback(cpu::Processor& processor_v, std::uint16_t callno_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	if (callno_v >= m_Callbacks.size() || m_Callbacks[callno_v].empty())
		return true;	
	auto registers_v = processor_v.GetRegisters();
	registers_v.rip = exit_v.VpContext.InstructionLength + exit_v.VpContext.Rip;
	auto ishandled_v = false;
	for (auto&& [_, callback_v] : m_Callbacks[callno_v]) {
		ishandled_v = true;
		if (callback_v(*m_Hypervisor, registers_v, processor_v, callno_v)) {			
			break; }}
	if (ishandled_v) {
		processor_v.SetRegisters(registers_v);
		return false;
	} 	
	__debugbreak();
	return true;
}

auto Manager::DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	auto& memory_v = m_Hypervisor->GetMemManager();
	auto cs_base_v = exit_v.VpContext.Cs.Base;
	auto pip_v = exit_v.VpContext.Rip;
	auto [opcode_v, number_v] = memory_v.FetchValue<std::uint8_t, std::uint16_t>(
		processor_v.GetIndex(), cs_base_v + pip_v - 3u, memory_v.kValidatedAddress);
	return DispatchCallback(processor_v, 0x68u != opcode_v ? kLastCallNumber : number_v, exit_v);
}

