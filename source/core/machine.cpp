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

#include <core/machine.hpp>

using core::Machine;

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

Machine::Machine(Config const& config_v)
{
	std::fill(std::begin(m_IoWrite), std::end(m_IoWrite), nullptr);
	std::fill(std::begin(m_IoFetch), std::end(m_IoFetch), nullptr);

	WIN32_ERROR_ASSERT(::WHvCreatePartition(&m_Partition));
	InitializePartitionProperties();
	config_v.ApplyBeforeSetup(*this);
	WIN32_ERROR_ASSERT(::WHvSetupPartition(m_Partition));
	config_v.ApplyAfterSetup(*this);
}

Machine::~Machine()
{
	if (m_Partition) {
		WHvDeletePartition(m_Partition);
		m_Partition = nullptr;
	}
	m_Memories.clear();
}

auto Machine::MapMemory(std::size_t index_v, std::uint64_t base_v, std::uint64_t size_v, std::uint32_t flags_v, std::uint64_t offset_v) -> void
{
	if (index_v >= m_Memories.size()) {
		throw std::invalid_argument("Invalid memory index");
	}

	auto address_v = m_Memories[index_v].Data() + offset_v;

	if (0u == size_v) {
		if (offset_v >= m_Memories[index_v].Size()) {
			throw std::invalid_argument("Invalid memory offset");
		}
		size_v = m_Memories[index_v].Size() - offset_v;
	}

	WIN32_ERROR_ASSERT(::WHvMapGpaRange(m_Partition, address_v, base_v, size_v, (WHV_MAP_GPA_RANGE_FLAGS)flags_v));
}

void Machine::UnmapMemory(std::uint64_t base_v, std::uint64_t size_v)
{
	WIN32_ERROR_ASSERT(::WHvUnmapGpaRange(m_Partition, base_v, size_v));
}

auto Machine::TranslateVirtualAddress(std::uint32_t index_v, std::uint64_t& inout_address_v, 
	WHV_TRANSLATE_GVA_FLAGS flags_v) const -> WHV_TRANSLATE_GVA_RESULT_CODE
{
	WHV_TRANSLATE_GVA_RESULT result_v { };
	auto const control0_v = GetRegister<std::uint64_t>(index_v, WHvX64RegisterCr0);
	static constexpr const std::uint64_t kPagingEnabled = 0x80000000u;
	if (!(control0_v & kPagingEnabled)) {
		return WHvTranslateGvaResultSuccess;
	}
	WIN32_ERROR_ASSERT(::WHvTranslateGva(m_Partition, index_v, 
		inout_address_v, flags_v, &result_v, &inout_address_v));
	return result_v.ResultCode;
}

auto Machine::ReadPhysical(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte> buffer_v, 
	WHV_CACHE_TYPE cache_control_v) const -> void 
{	
	WIN32_ERROR_ASSERT(::WHvReadGpaRange(m_Partition, index_v, address_v, WHV_ACCESS_GPA_CONTROLS{ 
		.CacheType = cache_control_v }, buffer_v.data(), buffer_v.size()));
}

auto core::Machine::WritePhysical(std::uint32_t index_v, std::uint64_t address_v, std::span<std::byte const> buffer_v, 
	WHV_CACHE_TYPE cache_control_v) const -> void 
{
	WIN32_ERROR_ASSERT(::WHvWriteGpaRange(m_Partition, index_v, address_v, WHV_ACCESS_GPA_CONTROLS{
		.CacheType = cache_control_v }, buffer_v.data(), buffer_v.size()));
}

auto Machine::InitializeProcessor(std::uint32_t index_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_Partition, index_v, 0u));
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v,
		InitialProcessorState::Names, 
		InitialProcessorState::Count, 
		InitialProcessorState::Values));
	auto const hypercall_v = IsVendorIntel() * 1u + IsVendorAMD() * 2u;
	SetRegister (index_v, WHvX64RegisterRax, WHV_REGISTER_VALUE{ .Reg64 = hypercall_v });
	m_Processors.emplace_back(index_v);
}

auto Machine::HandleIoOperation(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool {
	auto const& access_v = exit_v.IoPortAccess;
	auto rip_v = exit_v.VpContext.Rip + exit_v.VpContext.InstructionLength;
	SetRegister(index_v, WHvX64RegisterRip, rip_v);
	if (access_v.AccessInfo.IsWrite) {
		if (!m_IoWrite[access_v.PortNumber])
			return false;
		auto& device_v = *m_IoWrite[access_v.PortNumber];
		return device_v.PortWrite(access_v.PortNumber, 
			access_v.Rax, access_v.AccessInfo.AccessSize);		
	}
	else {
		if (!m_IoFetch[access_v.PortNumber])
			return false;
		auto& device_v = *m_IoFetch[access_v.PortNumber];
		auto value_v = (std::uint64_t)(- 1ull);
		auto const result_v = device_v.PortFetch(access_v.PortNumber, 
			value_v, access_v.AccessInfo.AccessSize);
		value_v = utils::crossover_bits(value_v, access_v.Rax, 
			access_v.AccessInfo.AccessSize*8u);
		SetRegister(index_v, WHvX64RegisterRax, value_v);
		return result_v;
	}
}

auto Machine::HandleHypercall(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool 
{	
	// Continue after VMCall when we return from the handler
	SetRegister(index_v, WHvX64RegisterRip, std::uint64_t(exit_v.VpContext.Rip + 
		exit_v.VpContext.InstructionLength));

	// Now we want to read 3 bytes directly preceding the VMCall instruction
	std::uint64_t push_address_v { exit_v.VpContext.Rip + exit_v.VpContext.Cs.Base - 3u };
	if (WHvTranslateGvaResultSuccess != TranslateVirtualAddress(index_v, push_address_v)) {
		// Can't translate the address ? something is very wrong here
		return false;
	}
	
	// Read the instruction
#pragma pack(push, 1)
	struct push16_type {	std::uint8_t icode; std::uint16_t value; };
#pragma pack(pop)
	
	auto registers_v = GetRegisters(index_v);
	if (auto const push_v = ReadPhysical<push16_type>(
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

auto Machine::HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool {
	if (exit_v.VpContext.Rflags & 0x200u) {
		// Interrupts enabled
		__debugbreak();
		return true;
	}

	return false;
}

auto Machine::Disassemble(std::ostream& output_v, std::uint32_t index_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void {
	capstone::instance capstone_v { cs_arch::CS_ARCH_X86, cs_mode::CS_MODE_16, {
		{ CS_OPT_SYNTAX, CS_OPT_SYNTAX_INTEL }, 
		{ CS_OPT_DETAIL, CS_OPT_ON } 
	}};

	
	std::uint8_t bytes_v[64];
	std::size_t remaining_bytes_v = std::size(bytes_v);

	while (count_v > 0u)
	{
		std::size_t next_byte_v { 0 };
		for (auto i = 0u; i < remaining_bytes_v; ++i) 
		{
			std::uint64_t address_v { virtual_address_v + i };
			if (WHvTranslateGvaResultSuccess!=
				TranslateVirtualAddress(index_v, address_v))
			{
				throw std::runtime_error("Unable to disassemble: TranslateVirtualAddress failed.");
			}
			bytes_v[next_byte_v] = ReadPhysical<std::uint8_t>(index_v, address_v);
			next_byte_v+=1u;
		}

		auto disassembly_v = capstone_v.disasm(bytes_v, virtual_address_v, count_v);

		for (auto&& instruction_v : disassembly_v)
		{
			std::string bytes_string_v;
			for (auto&& ibyte_v : bytes_v) {
				bytes_string_v += std::format("{:02x} ", ibyte_v);
			}

			std::cerr << std::format("{:08x} ({:08x}) : {:>20} : {:>9} {:>9}\n", 
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

auto Machine::HandleExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool {
	switch (exit_v.ExitReason) {
	case WHvRunVpExitReasonX64IoPortAccess: 
		HandleIoOperation(index_v, exit_v);		
		break;
	case WHvRunVpExitReasonX64Halt:
		return HandleHaltInstruction(index_v, exit_v);
	case WHvRunVpExitReasonHypercall:	
		return HandleHypercall(index_v, exit_v);
	case WHvRunVpExitReasonMemoryAccess: 
		__debugbreak();
		return false;
	default:
		Disassemble(std::cerr, index_v, exit_v.VpContext.Rip+exit_v.VpContext.Cs.Base, 20u);
		__debugbreak();
		return false;
	}
	return true;
}

auto Machine::RunVirtualProcessor(std::uint32_t index_v, std::stop_token token_v) -> void {
	WHV_RUN_VP_EXIT_CONTEXT exit_v;
	while (!token_v.stop_requested()) {
		std::memset(&exit_v, 0, sizeof(exit_v));
		WIN32_ERROR_ASSERT(::WHvRunVirtualProcessor(m_Partition, 
			index_v, &exit_v, sizeof(exit_v)));
		if (!HandleExit(index_v, exit_v)) {
			break;
		}
	}
}

auto Machine::Run() -> void
{
	std::vector<std::future<void>> futures_v;
	futures_v.reserve(m_Processors.size());
	for(auto&& processor_v : m_Processors) {		
		futures_v.emplace_back(std::async(std::launch::async,
			[this, processor_v, token_v = m_ProcessorBreak.get_token()] () mutable -> void {
				RunVirtualProcessor(processor_v, std::move(token_v));
			}));		
	}
	for(auto&& future_v : futures_v) {
		future_v.get();
	}
}

auto Machine::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Partition, code_v, data_v, size_v));
}

auto Machine::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Partition, code_v, data_v, size_v, &size_v));
}

auto core::Machine::MapIoRange(IOHandler& device_v, std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v) -> void {
	auto const end_v = base_v + size_v;
	for (auto port_v = base_v; port_v < end_v; port_v += 1u) {
		if (flags_v & kAccessWrite) {
			if (m_IoWrite[port_v]) 
				throw std::invalid_argument("Port already mapped");			
			m_IoWrite[port_v] = &device_v;
		}
		if (flags_v & kAccessFetch) {
			if (m_IoFetch[port_v])
				throw std::invalid_argument("Port already mapped");			
			m_IoFetch[port_v] = &device_v;
		}
	}
}

auto core::Machine::UnmapIoRange(std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v) -> void {
	auto const end_v = base_v + size_v;
	for (auto port_v = base_v; port_v < end_v; port_v += 1u) {
		if (flags_v & kAccessWrite) 
			m_IoWrite[port_v] = nullptr;
		if (flags_v & kAccessFetch) 
			m_IoFetch[port_v] = nullptr;
	}
}

auto core::Machine::MapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void
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

auto core::Machine::UnmapVcRange(VCHandler& handler_v, std::uint16_t base_v, std::uint16_t size_v) -> void
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

auto core::Machine::UnmapVcRange(std::uint16_t base_v, std::uint16_t size_v) -> void
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

auto core::Machine::GetRegisters(std::uint32_t index_v) const -> RegisterFile
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

auto core::Machine::SetRegisters(std::uint32_t index_v, RegisterFile const& registers_v) -> void
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
	values_v[18].Segment = { registers_v.cs_base, registers_v.cs_size, registers_v.cs, registers_v.cs_attr };
	values_v[19].Segment = { registers_v.ds_base, registers_v.ds_size, registers_v.ds, registers_v.ds_attr };
	values_v[20].Segment = { registers_v.es_base, registers_v.es_size, registers_v.es, registers_v.es_attr };
	values_v[21].Segment = { registers_v.fs_base, registers_v.fs_size, registers_v.fs, registers_v.fs_attr };
	values_v[22].Segment = { registers_v.gs_base, registers_v.gs_size, registers_v.gs, registers_v.gs_attr };
	values_v[23].Segment = { registers_v.ss_base, registers_v.ss_size, registers_v.ss, registers_v.ss_attr };	
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v, RegisterFile::Layout, std::size(RegisterFile::Layout), values_v.data()));
}

auto core::Machine::IsVendorIntel() -> bool
{
	auto const vendor_v = GetCapability<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorIntel;
}

auto core::Machine::IsVendorAMD() -> bool
{
	auto const vendor_v = GetCapability<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorAmd || vendor_v == WHvProcessorVendorHygon;
}

auto core::Machine::GetCapability(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto Machine::InitializePartitionProperties() -> void {
	SetProperty(WHvPartitionPropertyCodeExceptionExitBitmap, std::uint64_t{ 0 });
	SetProperty(WHvPartitionPropertyCodeExtendedVmExits, WHV_EXTENDED_VM_EXITS{ .HypercallExit = 1 });
	SetProperty(WHvPartitionPropertyCodeProcessorFeatures, WHV_PROCESSOR_FEATURES{ .LahfSahfSupport = 1 });
}