#include <win32/error.hpp>
#include <utils/bitmanip.hpp>

#include <stop_token>
#include <stdexcept>
#include <algorithm>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <array>

#include "machine.hpp"

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

auto Machine::InitializeProcessor(std::uint32_t index_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvCreateVirtualProcessor(m_Partition, index_v, 0u));
	WIN32_ERROR_ASSERT(::WHvSetVirtualProcessorRegisters(m_Partition, index_v,
		InitialProcessorState::Names, 
		InitialProcessorState::Count, 
		InitialProcessorState::Values));
	auto const hypercall_v = HasVMCALL() * 1u + HasVMMCALL() * 2u;
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

auto Machine::HandleHaltInstruction(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool {
	if (exit_v.VpContext.Rflags & 0x200u) {
		// Interrupts enabled
		__debugbreak();
		return true;
	}

	return false;
}

auto Machine::HandleExit(std::uint32_t index_v, WHV_RUN_VP_EXIT_CONTEXT& exit_v) -> bool {
	switch (exit_v.ExitReason) {
	case WHvRunVpExitReasonX64IoPortAccess: 
		HandleIoOperation(index_v, exit_v);		
		break;
	case WHvRunVpExitReasonX64Halt:
		return HandleHaltInstruction(index_v, exit_v);
	case WHvRunVpExitReasonHypercall:
		__debugbreak();
		return false;
	case WHvRunVpExitReasonMemoryAccess: 
		__debugbreak();
		return false;
	default:
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
}

auto Machine::SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void const* data_v, std::uint32_t size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvSetPartitionProperty(m_Partition, code_v, data_v, size_v));
}

auto Machine::GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, void* data_v, std::uint32_t& size_v) -> void
{
	WIN32_ERROR_ASSERT(::WHvGetPartitionProperty(m_Partition, code_v, data_v, size_v, &size_v));
}

auto core::Machine::MapIoRange(IODevice& device_v, std::uint16_t base_v, std::uint16_t size_v, std::uint32_t flags_v) -> void {
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

auto core::Machine::HasVMCALL() -> bool
{
	auto const vendor_v = GetCapability<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorIntel;
}

auto core::Machine::HasVMMCALL() -> bool
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