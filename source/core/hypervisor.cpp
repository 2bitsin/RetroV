#include <win32/error.hpp>
#include <utils/bitmanip.hpp>
#include <utils/capstone.hpp>

#include <stop_token>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <format>
#include <vector>
#include <ranges>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <array>

#include <core/hypervisor.hpp>

using core::Hypervisor;

Hypervisor::Hypervisor(Config const& config_v)
	:	m_MemPool{ }
	,	m_MemManager{ *this }
	, m_IoManager{ *this }
	,	m_VcManager { *this }
	,	m_Processors{ }
	,	m_Partition{ nullptr }
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Partition));
	InitializePartition();
	config_v.ApplyBeforeSetup(*this);
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_Partition));
	config_v.ApplyAfterSetup(*this);
}

Hypervisor::~Hypervisor()
{	
	if (m_Partition) {
		WHvDeletePartition(m_Partition);
		m_Partition = nullptr;
	}
}

auto Hypervisor::GetParitionHandle() -> WHV_PARTITION_HANDLE
{
	return m_Partition;
}

auto Hypervisor::GetMemoryPool() -> mem::Pool&
{
	return m_MemPool;
}

auto Hypervisor::GetMemoryManager() -> mem::Manager&
{
	return m_MemManager;
}

auto Hypervisor::GetIoManager() -> io::Manager&
{
	return m_IoManager;
}

auto Hypervisor::GetVcManager() -> vmc::Manager&
{
	return m_VcManager;
}

auto Hypervisor::GetProcessor(std::uint32_t index_v)->cpu::Processor&
{
	return m_Processors.at(index_v);
}

auto Hypervisor::DispatchHalt(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool 
{
	if (exit_v.VpContext.Rflags & 0x200u) {
		// Interrupts enabled
		__debugbreak();
		return true;
	}

	return false;
}

auto Hypervisor::PrintRegisters(std::ostream& output_v, core::RegisterFile const& R) -> void {
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

auto Hypervisor::Disassemble(std::ostream& output_v, cpu::Processor& processor_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void
{
	capstone::instance capstone_v { cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_16, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL }, 
		{ CS_OPT_DETAIL, CS_OPT_ON } 
	}};

	auto& memory_v = GetMemoryManager();		
	std::byte bytes_v[64];
	std::size_t remaining_bytes_v = std::size(bytes_v);

	while (count_v > 0u)
	{
		auto& memory_v = GetMemoryManager();
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

auto Hypervisor::DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool
{	
	switch (exit_v.ExitReason) 
	{
	case WHvRunVpExitReasonX64Halt:
		return DispatchHalt(processor_v, exit_v);
	case WHvRunVpExitReasonX64IoPortAccess:
		return m_IoManager.DispatchExit(processor_v, exit_v);
	case WHvRunVpExitReasonHypercall:	
		return m_VcManager.DispatchExit(processor_v, exit_v);
	default:
		__debugbreak();
		PrintRegisters(std::cerr, processor_v.GetRegisters());
		Disassemble(std::cerr, processor_v, exit_v.VpContext.Rip+exit_v.VpContext.Cs.Base, 20u);
		__debugbreak();
		return false;
	}
	return true;
}

auto Hypervisor::NextInstruction(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> void {	
	processor_v.SetRegister(WHvX64RegisterRip, exit_v.VpContext.Rip + exit_v.VpContext.InstructionLength);
}


auto Hypervisor::Run() -> void
{
	auto& processor_v = m_Processors[0];
	auto exit_v = processor_v.Run();
		
}

auto Hypervisor::InitializePartition() -> void
{
	auto scheduler_features_v = GetCapability<WHV_CAPABILITY_PROCESSOR_FREQUENCY_CAP>(WHvCapabilityCodeProcessorFrequencyCap);

	SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, std::uint64_t{ 0x40u });
	SetProperty(WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .ExceptionExit = 1, .HypercallExit = 1 });
	SetProperty(WHvPartitionPropertyCodeProcessorFeatures, WHV_PROCESSOR_FEATURES{ .LahfSahfSupport = 1 });
}

auto Hypervisor::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Partition, code_v, data_v, size_v));
}

auto Hypervisor::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Partition, code_v, data_v, size_v, &size_v));
}

auto Hypervisor::GetCapability(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto Hypervisor::InitializeProcessor(std::uint32_t index_v) -> void
{
	m_Processors.emplace_back(*this, index_v);
}
