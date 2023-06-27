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

struct InitialProcessorState
{
	static constexpr const WHV_REGISTER_NAME Names[] = {
		/*  0 */ WHvX64RegisterRflags,
		/*  1 */ WHvX64RegisterRip,
		/*  2 */ WHvX64RegisterCs,
		/*  3 */ WHvX64RegisterDs,
		/*  4 */ WHvX64RegisterEs,
		/*  5 */ WHvX64RegisterSs,
		/*  6 */ WHvX64RegisterFs,
		/*  7 */ WHvX64RegisterGs,
		/*  8 */ WHvX64RegisterIdtr,
		/*  9 */ WHvX64RegisterGdtr,
		/*  A */ WHvX64RegisterRax,
		/*  B */ WHvX64RegisterRbx,
		/*  C */ WHvX64RegisterRcx,
		/*  D */ WHvX64RegisterRdx,
		/*  E */ WHvX64RegisterRsi,
		/*  F */ WHvX64RegisterRdi,
		/* 10 */ WHvX64RegisterRbp,
		/* 11 */ WHvX64RegisterRsp,
		/* 12 */ WHvX64RegisterR8,
		/* 13 */ WHvX64RegisterR9,
		/* 14 */ WHvX64RegisterR10,
		/* 15 */ WHvX64RegisterR11,
		/* 16 */ WHvX64RegisterR12,
		/* 17 */ WHvX64RegisterR13,
		/* 18 */ WHvX64RegisterR14,
		/* 19 */ WHvX64RegisterR15,
		/* 1A */ WHvX64RegisterCr0,
		/* 1B */ WHvX64RegisterCr2,
		/* 1C */ WHvX64RegisterCr3,
		/* 1D */ WHvX64RegisterCr4,
	};

	static constexpr const WHV_REGISTER_VALUE Values[] =
	{
		/* FLAGS */ {.Reg64 = 0x0000000000000002u },
		/* RIP   */ {.Reg64 = 0x000000000000FFF0u },
		/* CS    */ {.Segment = {.Base = 0xf0000u, .Limit = 0xFFFFu, .Selector = 0xF000u, .Attributes = 0x009Eu } },
		/* DS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* ES    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* SS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* FS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* GS    */ {.Segment = {.Base = 0x00000u, .Limit = 0xFFFFu, .Selector = 0x0000u, .Attributes = 0x0082u } },
		/* GDTR  */ {.Table = {.Limit = 0x03FFu, .Base = 0x00000000u  } },
		/* IDTR  */ {.Table = {.Limit = 0x0000u, .Base = 0x00000000u  } },
		/* RAX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RBX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RCX   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RDX   */ {.Reg64 = 0x0000'0000'0000'0000u},		
		/* RSI   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RDI   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RSP   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* RBP   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R8    */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R9    */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R10   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R11   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R12   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R13   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R14   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* R15   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR0   */ {.Reg64 = 0x0000'0000'6000'0010u},
		/* CR2   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR3   */ {.Reg64 = 0x0000'0000'0000'0000u},
		/* CR4   */ {.Reg64 = 0x0000'0000'0000'0000u}
	};

	static constexpr const std::size_t Count = std::min(std::size(Names), std::size(Values));
};

Hypervisor::Hypervisor(Config const& config_v)
	:	m_MemoryPool{ }
	,	m_MemoryManager{ *this }
	, m_IoManager{ *this }
	,	m_Processors{ }
	,	m_Partition{ nullptr }
{
	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Partition));
	InitializePartitionProperties();
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

auto Hypervisor::GetCpuIndexes() -> std::span<std::uint32_t const>
{
	return m_Processors;
}

auto Hypervisor::GetMemoryPool() -> memory::Pool&
{
	return m_MemoryPool;
}

auto core::Hypervisor::GetMemoryManager() -> memory::Manager&
{
	return m_MemoryManager;
}

auto core::Hypervisor::GetIoManager() -> io::Manager&
{
	return m_IoManager;
}

auto Hypervisor::InitializeProcessor(std::uint32_t index_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_Partition, index_v, 0u));
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v,
		InitialProcessorState::Names, 
		InitialProcessorState::Count, 
		InitialProcessorState::Values));
	m_Processors.emplace_back(index_v);
}


auto Hypervisor::HandleHypercall(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool 
{	
	// Continue after VMCall when we return from the handler
	SetRegister(index_v, WHvX64RegisterRip, std::uint64_t(exit_v.VpContext.Rip + 
		exit_v.VpContext.InstructionLength));

	auto& memory_v = GetMemoryManager();
	// Now we want to read 3 bytes directly preceding the VMCall instruction
	std::uint64_t push_address_v { exit_v.VpContext.Rip + exit_v.VpContext.Cs.Base - 3u };
	if (WHvTranslateGvaResultSuccess != memory_v.VirtualToPhysical(index_v, push_address_v)) {
		// Can't translate the address ? something is very wrong here
		return false;
	}
	
	// Read the instruction
#pragma pack(push, 1)
	struct push16_type {	std::uint8_t icode; std::uint16_t value; };
#pragma pack(pop)
	
	auto registers_v = GetRegisters(index_v);
	if (auto const push_v = memory_v.FetchValue<push16_type>(
		index_v, push_address_v); 
		push_v.icode == 0x68u) 
	{
		if (push_v.value >= m_VmmCall.size() || m_VmmCall[push_v.value].empty()) {
			std::cerr << std::format("WARNING! No handler for VMCall({:#x})\n", push_v.value);
			__debugbreak();
			return false;
		}
		bool was_handled_v = false;
		for (auto& handler_v : m_VmmCall[push_v.value]) {
			if (handler_v->VMCall (*this, index_v, registers_v, push_v.value)) {
				was_handled_v = true;
				break; 
			}
		}
		if (!was_handled_v) {
			std::cerr << std::format("WARNING! None of the VMCall({:#x}) handlers handled the call!\n", push_v.value);
			__debugbreak();
			return false;
		}
		SetRegisters(index_v, registers_v);		
		return true;
	} else {
		std::cerr << std::format("WARNING! Weren't able to determine the VMCall number!\n", push_v.value);
		__debugbreak();
	}

	return false;
}

auto Hypervisor::HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool 
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

auto Hypervisor::Disassemble(std::ostream& output_v, std::uint32_t index_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void 
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
		memory_v.Fetch(index_v, virtual_address_v, next_buffer_v, memory_v.kVirtualAddress);

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

auto Hypervisor::HandleExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool 
{
	switch (exit_v.ExitReason) {
	case WHvRunVpExitReasonX64Halt:
		return HandleHaltInstruction(index_v, exit_v);
	case WHvRunVpExitReasonX64IoPortAccess:
		return m_IoManager.DispatchIoExit(index_v, exit_v);
	case WHvRunVpExitReasonHypercall:	
		return HandleHypercall(index_v, exit_v);
	default:
		PrintRegisters(std::cerr, GetRegisters(index_v));
		Disassemble(std::cerr, index_v, exit_v.VpContext.Rip+exit_v.VpContext.Cs.Base, 20u);
		__debugbreak();
		return false;
	}
	return true;
}

auto Hypervisor::RunVirtualProcessor(std::uint32_t index_v, std::stop_token token_v) -> void {
	try
	{
		WHV_RUN_VP_EXIT_CONTEXT exit_v;
		while (!token_v.stop_requested()) {
			std::memset(&exit_v, 0, sizeof(exit_v));
			WIN32_ERROR_ASSERT(::WHvRunVirtualProcessor(m_Partition, 
				index_v, &exit_v, sizeof(exit_v)));
			if (!HandleExit(index_v, exit_v)) {
				break;
			}
		}
	} catch (std::exception const& ex) {
		std::cerr << __func__ << ": " << ex.what() << "\n";
		throw;
	}
}

auto Hypervisor::Run() -> void
{
	std::vector<std::future<void>> futures_v;
	futures_v.reserve(m_Processors.size());
	auto launch_v = std::launch::deferred;
	for(auto&& processor_v : m_Processors) {
		futures_v.emplace_back(std::async(launch_v,
			[this, processor_v, token_v = m_ProcessorBreak.get_token()] () mutable -> void {
				RunVirtualProcessor(processor_v, std::move(token_v));
			}));		
		launch_v = std::launch::async;
	}
	for(auto&& future_v : futures_v) {
		future_v.get();
	}
}

auto Hypervisor::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Partition, code_v, data_v, size_v));
}

auto Hypervisor::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Partition, code_v, data_v, size_v, &size_v));
}


auto core::Hypervisor::MapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void
{
	auto const end_v = base_v + size_v;
	if (end_v > 0x10000u) {
		throw std::invalid_argument("Invalid range");
	}
	if (end_v >= m_VmmCall.size()) {
		m_VmmCall.resize(end_v);
	}

	for (auto index_v = base_v; index_v < end_v; index_v += 1u) 
	{
		auto& slot_v = m_VmmCall[index_v];
		auto offset_v = std::ranges::find(slot_v, &handler_v);
		if (offset_v != slot_v.end()) {
			throw std::invalid_argument("Handler already mapped");
		}
		slot_v.emplace(slot_v.begin(), &handler_v);
	}
}

auto core::Hypervisor::UnmapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void
{
	auto const end_v = base_v + size_v;
	if (end_v > 0x10000u) {
		throw std::invalid_argument("Invalid range");
	}
	auto const last_slot_v = std::min<std::size_t>(end_v, m_VmmCall.size());
	for (auto index_v = base_v; index_v < last_slot_v; index_v += 1u) 
	{
		auto& slot_v = m_VmmCall[index_v];
		auto offset_v = std::ranges::find(slot_v, &handler_v);
		if (offset_v == slot_v.end())
			continue;		
		slot_v.erase(offset_v);
	}
}

auto core::Hypervisor::UnmapVcRange(std::uint16_t base_v, std::uint16_t size_v) -> void
{
	auto const end_v = base_v + size_v;
	if (end_v > 0x10000u) {
		throw std::invalid_argument("Invalid range");
	}
	auto const last_slot_v = std::min<std::size_t>(end_v, m_VmmCall.size());
	for (auto index_v = base_v; index_v < last_slot_v; index_v += 1u) 
	{
		auto& slot_v = m_VmmCall[index_v];
		slot_v.clear();
	}
}

auto core::Hypervisor::GetRegisters(std::uint32_t index_v) const -> RegisterFile
{
	std::vector<WHV_REGISTER_VALUE> values_v;
	values_v.resize(std::size(RegisterFile::Layout));
	WIN32_ERROR_ASSERT(::WHvGetVirtualProcessorRegisters(m_Partition, index_v,
		RegisterFile::Layout, std::size(RegisterFile::Layout), values_v.data()));	
	return RegisterFile 
	{ 
		.rax			= values_v[0].Reg64,
		.rbx			= values_v[1].Reg64,
		.rcx			= values_v[2].Reg64,
		.rdx			= values_v[3].Reg64,	
		.rsi			= values_v[4].Reg64,
		.rdi			= values_v[5].Reg64,
		.rbp			= values_v[6].Reg64,
		.rsp			= values_v[7].Reg64,
		.r8				= values_v[8].Reg64,
		.r9				= values_v[9].Reg64,
		.r10			= values_v[10].Reg64,
		.r11			= values_v[11].Reg64,
		.r12			= values_v[12].Reg64,
		.r13			= values_v[13].Reg64,
		.r14			= values_v[14].Reg64,
		.r15			= values_v[15].Reg64,
		.rip			= values_v[16].Reg64,
		.rflags		= values_v[17].Reg64,

		.cs_base	= values_v[18].Segment.Base,
		.cs_size	= values_v[18].Segment.Limit,
		.cs				= values_v[18].Segment.Selector,
		.cs_attr	= values_v[18].Segment.Attributes,

		.ds_base	= values_v[19].Segment.Base,
		.ds_size	= values_v[19].Segment.Limit,
		.ds				= values_v[19].Segment.Selector,
		.ds_attr	= values_v[19].Segment.Attributes,

		.es_base	= values_v[20].Segment.Base,
		.es_size	= values_v[20].Segment.Limit,
		.es				= values_v[20].Segment.Selector,
		.es_attr	= values_v[20].Segment.Attributes,

		.fs_base	= values_v[21].Segment.Base,
		.fs_size	= values_v[21].Segment.Limit,
		.fs				= values_v[21].Segment.Selector,
		.fs_attr	= values_v[21].Segment.Attributes,

		.gs_base	= values_v[22].Segment.Base,
		.gs_size	= values_v[22].Segment.Limit,
		.gs				= values_v[22].Segment.Selector,
		.gs_attr	= values_v[22].Segment.Attributes,

		.ss_base	= values_v[23].Segment.Base,
		.ss_size	= values_v[23].Segment.Limit,
		.ss				= values_v[23].Segment.Selector,
		.ss_attr	= values_v[23].Segment.Attributes	
	};
}

auto core::Hypervisor::SetRegisters(std::uint32_t index_v, RegisterFile const& registers_v) -> void
{
	std::vector<WHV_REGISTER_VALUE> values_v;
	values_v.resize(std::size(RegisterFile::Layout));
	values_v[ 0].Reg64 = registers_v.rax;
	values_v[ 1].Reg64 = registers_v.rbx;
	values_v[ 2].Reg64 = registers_v.rcx;
	values_v[ 3].Reg64 = registers_v.rdx;
	values_v[ 4].Reg64 = registers_v.rsi;
	values_v[ 5].Reg64 = registers_v.rdi;
	values_v[ 6].Reg64 = registers_v.rbp;
	values_v[ 7].Reg64 = registers_v.rsp;
	values_v[ 8].Reg64 = registers_v.r8;
	values_v[ 9].Reg64 = registers_v.r9;
	values_v[10].Reg64 = registers_v.r10;
	values_v[11].Reg64 = registers_v.r11;
	values_v[12].Reg64 = registers_v.r12;
	values_v[13].Reg64 = registers_v.r13;
	values_v[14].Reg64 = registers_v.r14;
	values_v[15].Reg64 = registers_v.r15;
	values_v[16].Reg64 = registers_v.rip;
	values_v[17].Reg64 = registers_v.rflags;
	values_v[18].Segment = { .Base=registers_v.cs_base, .Limit=registers_v.cs_size, .Selector=registers_v.cs, .Attributes=registers_v.cs_attr };
	values_v[19].Segment = { .Base=registers_v.ds_base, .Limit=registers_v.ds_size, .Selector=registers_v.ds, .Attributes=registers_v.ds_attr };
	values_v[20].Segment = { .Base=registers_v.es_base, .Limit=registers_v.es_size, .Selector=registers_v.es, .Attributes=registers_v.es_attr };
	values_v[21].Segment = { .Base=registers_v.fs_base, .Limit=registers_v.fs_size, .Selector=registers_v.fs, .Attributes=registers_v.fs_attr };
	values_v[22].Segment = { .Base=registers_v.gs_base, .Limit=registers_v.gs_size, .Selector=registers_v.gs, .Attributes=registers_v.gs_attr };
	values_v[23].Segment = { .Base=registers_v.ss_base, .Limit=registers_v.ss_size, .Selector=registers_v.ss, .Attributes=registers_v.ss_attr };	
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v, RegisterFile::Layout, std::size(RegisterFile::Layout), values_v.data()));
}

auto Hypervisor::IsVendorIntel() -> bool
{
	auto const vendor_v = GetCapability<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorIntel;
}

auto Hypervisor::IsVendorAMD() -> bool
{
	auto const vendor_v = GetCapability<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorAmd || vendor_v == WHvProcessorVendorHygon;
}

auto Hypervisor::GetCapability(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto Hypervisor::InitializePartitionProperties() -> void 
{
	SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, std::uint64_t{ 0 });
	SetProperty(WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .HypercallExit = 1 });
	SetProperty(WHvPartitionPropertyCodeProcessorFeatures, WHV_PROCESSOR_FEATURES{ .LahfSahfSupport = 1 });
}