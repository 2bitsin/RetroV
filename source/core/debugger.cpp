#include <core/debugger.hpp>
#include <core/machine.hpp>

#include <utils/logger.hpp>
#include <bios/vmcall.h>

#include <charconv>
#include <cassert>

using core::Debugger;

Debugger::Debugger(Machine& machine_v)
	: m_Machine{ machine_v }
{}

auto Debugger::Reset() -> void
{
	std::unique_lock lock{ m_Mutex };
	m_Buffer.clear();
}

auto Debugger::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	return ERROR_ACCESS_DENIED;
}

auto Debugger::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
  return ERROR_ACCESS_DENIED;
}

auto Debugger::Hypercall_UnrealModeEnable(Processor const& vcpu_v, bool enable_v) -> std::int32_t 
{	
	static constexpr const WHV_REGISTER_NAME name_v[] = {
		WHvX64RegisterDs,
		WHvX64RegisterEs,
		WHvX64RegisterFs,
		WHvX64RegisterGs
	};
	WHV_REGISTER_VALUE value_v[4];

	if (enable_v)
	{
		s_log.UnrealModeEnabled(vcpu_v.GetIndex());
		vcpu_v.GetRegisters(name_v, value_v);
		for (auto&& v : value_v) {
			v.Segment.Base = 0u;
			v.Segment.Limit = 0xFFFFFFFFu;
			v.Segment.Attributes = 0xCF93u;
		}
		vcpu_v.SetRegisters(name_v, value_v);
	}
	else 
	{
		s_log.UnrealModeDisabled(vcpu_v.GetIndex());
		vcpu_v.GetRegisters(name_v, value_v);
		for (auto&& v : value_v) {			
		v.Segment.Limit = 0xFFFFu;
			v.Segment.Attributes = 0x0093u;			
		}
		vcpu_v.SetRegisters(name_v, value_v);
	}
	return S_OK;
}

auto Debugger::Hypercall_WriteLogString(Processor const& vcpu_v, std::uint64_t address_v, std::uint64_t length_v) -> std::int32_t
{
	using utils::logger;
	std::vector<std::byte> output_v;
	auto status_v = m_Machine.GetMemory().FetchMemory(vcpu_v, address_v, length_v, output_v);
	if (status_v != ERROR_SUCCESS)
		return status_v;
	WriteLogString(vcpu_v, { (char*)output_v.data(), output_v.size() });
	return ERROR_SUCCESS;
}

auto Debugger::Hypercall_DebuggerBreak(Processor const& vcpu_v, std::uint64_t lin_v, std::uint16_t seg_v, std::uint64_t off_v) -> std::int32_t
{
	using utils::logger;
	s_log.DebugTrap(vcpu_v.GetIndex(), lin_v, seg_v, off_v);
	__debugbreak();
  return ERROR_SUCCESS;
}

auto Debugger::Hypercall_WriteLogChar(Processor const& vcpu_v, char value_v) -> std::int32_t
{
	WriteLogString(vcpu_v, { &value_v, 1u });
	return ERROR_SUCCESS;
}

auto Debugger::WriteLogString(Processor const& vcpu_v, std::string_view message_v) -> void
{
	std::lock_guard lock_v{ m_Mutex };
	for (auto value_v: message_v) {	
		if (value_v != '\n') {
			m_Buffer.push_back(value_v);
			continue;
		}
		utils::logger::debug(utils::logger::deflog, "CPU[{}] : {}", vcpu_v.GetIndex(), m_Buffer);
		m_Buffer.clear();
	}
}

auto Debugger::Hypercall(Processor const& vcpu_v, HypercallContext const& context_v) -> std::int32_t
{
	using namespace win32::regs;

	auto const& hypercall_v = context_v.Hypercall;
	auto const& vpcontext_v = context_v.VpContext;	

	if (vpcontext_v.ExecutionState.Cpl != 0)
		return ERROR_ACCESS_DENIED;
	auto const seg_v = vcpu_v.GetRegisters<Cs, Ds, Es>();
	if (seg_v.cs.default_32bit)
		return ERROR_ACCESS_DENIED;

	switch (context_v.Function) 
	{
	case HYPERCALL_DEBUG_TOGGLE_UNREAL_MODE:
		return Hypercall_UnrealModeEnable(vcpu_v, 
			!!(hypercall_v.Rbx&1u));
	case HYPERCALL_DEBUG_WRITE_LOG_CHAR:     
		return Hypercall_WriteLogChar(vcpu_v, 
			hypercall_v.Rbx&0xFFu);
	case HYPERCALL_DEBUG_WRITE_LOG_STRING: 	
		return Hypercall_WriteLogString(vcpu_v,
			seg_v.ds.base + (hypercall_v.Rsi&0xFFFFu), 
			hypercall_v.Rcx&0xFFFFu);
	case HYPERCALL_DEBUG_DEBUGGER_BREAK: 
		return Hypercall_DebuggerBreak(vcpu_v, vpcontext_v.Cs.Base + vpcontext_v.Rip, 
			vpcontext_v.Cs.Selector, vpcontext_v.Rip);
	}
  return ERROR_ACCESS_DENIED;
}

