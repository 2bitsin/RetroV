#include <core/vmc/manager.hpp>
#include <core/hypervisor.hpp>

using core::vmc::Manager;

Manager::Manager(core::Hypervisor& hypervisor_v)
	: m_Hypervisor(hypervisor_v)
{
	m_Callbacks.reserve(kLastCallNumber+1u);
}

Manager::~Manager()
{}

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

#include <iostream>
auto Manager::DispatchCallback(std::uint32_t index_v, std::uint16_t callno_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	//std::cerr << "Dispatching callback " << std::hex << callno_v << std::endl;
	if (callno_v >= m_Callbacks.size() || m_Callbacks[callno_v].empty())
		return true;
	auto& processor_v = m_Hypervisor.GetProcessor(index_v);
	auto registers_v = processor_v.GetRegisters();
	registers_v.rip = exit_v.VpContext.InstructionLength + exit_v.VpContext.Rip;
	auto ishandled_v = false;
	for (auto&& [_, callback_v] : m_Callbacks[callno_v]) {
		ishandled_v = true;
		if (callback_v(m_Hypervisor, registers_v, index_v, callno_v)) {			
			break; }}
	if (ishandled_v) {
		processor_v.SetRegisters(registers_v);
		return false;
	} 	
	__debugbreak();
	return true;
}

auto Manager::DispatchExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{
	auto& memory_v = m_Hypervisor.GetMemoryManager();
	auto cs_base_v = exit_v.VpContext.Cs.Base;
	auto pip_v = exit_v.VpContext.Rip;
	auto [opcode_v, number_v] = memory_v.FetchValue<std::uint8_t, std::uint16_t>(
		index_v, cs_base_v + pip_v - 3u, memory_v.kValidatedAddress);		
	return DispatchCallback(index_v, 0x68u != opcode_v ? kLastCallNumber : number_v, exit_v);
}