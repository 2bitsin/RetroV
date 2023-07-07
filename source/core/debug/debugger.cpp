#include <core/debug/debugger.hpp>

#include <core/hypervisor.hpp>

#include <iostream>
#include <format>
#include <algorithm>

using core::debug::Debugger;

Debugger::Debugger(Hypervisor& hypervisor_v)
	: m_Hypervisor(&hypervisor_v) 
	, m_Capstone16(cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_16, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL },
		{ CS_OPT_DETAIL, CS_OPT_ON }})
	, m_Capstone32(cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_32, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL },
		{ CS_OPT_DETAIL, CS_OPT_ON }})
	, m_Capstone64(cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_64, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL },
		{ CS_OPT_DETAIL, CS_OPT_ON }})
{}

Debugger::Debugger(Debugger&& prev_v) noexcept 
	: m_Hypervisor(std::exchange(prev_v.m_Hypervisor, nullptr))
	, m_Capstone16(std::move(prev_v.m_Capstone16))
	, m_Capstone32(std::move(prev_v.m_Capstone32))
	, m_Capstone64(std::move(prev_v.m_Capstone64))
{}

auto Debugger::operator = (Debugger&& prev_v) noexcept -> Debugger& {
	if (this==&prev_v) 
		return *this;
	auto tmp_v{ std::move(prev_v) };
	tmp_v.Swap(*this);
	return *this;
}

auto Debugger::Swap(Debugger& other_v) noexcept -> void {
	std::swap(m_Hypervisor, other_v.m_Hypervisor);
	std::swap(m_Capstone16, other_v.m_Capstone16);
	std::swap(m_Capstone32, other_v.m_Capstone32);
	std::swap(m_Capstone64, other_v.m_Capstone64);
}

auto Debugger::PrintRegisters(std::ostream& output_v, core::RegisterFile const& R) -> void {
	// Print the registers in a nice table
	output_v
		<< std::format("RAX: {:#018x}\n", R.rax)
		<< std::format("RBX: {:#018x}\n", R.rbx)
		<< std::format("RCX: {:#018x}\n", R.rcx)
		<< std::format("RDX: {:#018x}\n", R.rdx)
		<< std::format("RSI: {:#018x}\n", R.rsi)
		<< std::format("RDI: {:#018x}\n", R.rdi)
		<< std::format("RBP: {:#018x}\n", R.rbp)
		<< std::format("RSP: {:#018x}\n", R.rsp)
		<< std::format("R8:  {:#018x}\n", R.r8)
		<< std::format("R9:  {:#018x}\n", R.r9)
		<< std::format("R10: {:#018x}\n", R.r10)
		<< std::format("R11: {:#018x}\n", R.r11)
		<< std::format("R12: {:#018x}\n", R.r12)
		<< std::format("R13: {:#018x}\n", R.r13)
		<< std::format("R14: {:#018x}\n", R.r14)
		<< std::format("R15: {:#018x}\n", R.r15)
		<< std::format("RIP: {:#018x}\n", R.rip)
		<< std::format("RFLAGS: {:#016x}\n", R.rflags)
		<< std::format("CS: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.cs, R.cs_size, R.cs_base, R.cs_attr)
		<< std::format("DS: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.ds, R.ds_size, R.ds_base, R.ds_attr)
		<< std::format("ES: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.es, R.es_size, R.es_base, R.es_attr)
		<< std::format("FS: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.fs, R.fs_size, R.fs_base, R.fs_attr)
		<< std::format("GS: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.gs, R.gs_size, R.gs_base, R.gs_attr)
		<< std::format("SS: {:#06x} LIMIT:{:#010x} BASE:{:#018x} ATTR:{:#06x}\n", R.ss, R.ss_size, R.ss_base, R.ss_attr);
}

auto Debugger::Disassemble(std::ostream& output_v, Processor& processor_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void
{
	capstone::instance capstone_v { cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_16, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL },
		{ CS_OPT_DETAIL, CS_OPT_ON }
	}};

	auto& memory_v = m_Hypervisor->GetMemManager();
	std::byte bytes_v[64];
	std::size_t remaining_bytes_v = std::size(bytes_v);

	while (count_v > 0u)
	{
		auto& memory_v = m_Hypervisor->GetMemManager();
		auto next_buffer_v = std::span<std::byte>{ bytes_v }.first(remaining_bytes_v);
		memory_v.Fetch(processor_v.GetIndex(), virtual_address_v, next_buffer_v, memory_v.kVirtualAddress);

		auto disassembly_v = capstone_v.disasm(bytes_v, virtual_address_v, count_v);

		for (auto&& instruction_v : disassembly_v)
		{
			std::string bytes_string_v;
			for (auto&& ibyte_v : instruction_v.bytes()) {
				bytes_string_v += std::format("{:02x} ", (std::uint8_t)ibyte_v);
			}

			std::cerr << std::format("{:08x} ({:08x}) : {:<20} : {:<9} {:<9}\n",
				virtual_address_v, instruction_v.address(), bytes_string_v,
				instruction_v.mnemonic_string(),
				instruction_v.operands_string());

			virtual_address_v += instruction_v.bytes().size();
			remaining_bytes_v -= instruction_v.bytes().size();
			count_v -= 1u;
		}

		if (remaining_bytes_v > 0u) {
			std::memcpy(&bytes_v[0], &bytes_v[std::size(bytes_v) - remaining_bytes_v], remaining_bytes_v);
			remaining_bytes_v = std::size(bytes_v);
		}
	}
}
