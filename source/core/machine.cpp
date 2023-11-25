#include <system_error>
#include <filesystem>
#include <algorithm>
#include <ranges>


#include <win32/whvcapabilities.hpp>
#include <win32/whvregisters.hpp>

#include <core/machine.hpp>
#include <core/constants.hpp>

#include <utils/region.hpp>
#include <utils/algorithm.hpp>
#include <utils/literals.hpp>
#include <utils/logger.hpp>
#include <utils/paths.hpp>
#include <utils/span.hpp>
#include <utils/validate.hpp>

#include <SDL2/SDL.h>

#undef main

using namespace size_literals;

using core::Machine;

Machine::Machine(Configuration const& config_v)	
	: m_Partition		{ nullptr }
	, m_Processor		{ *this, 0u }
	, m_Memory			{ *this }
	, m_LegacyPic		{ *this, 0u }
	, m_Debugger		{ *this }
	, m_VideoDevice { *this }
	, m_Display			{ *this }
{
	ConfigurePartition(config_v);
	ConfigureMemory(config_v);	
	m_LegacyPic.Initialize();
	m_Display.Initialize();
	m_VideoDevice.Initialize(config_v);
}

void Machine::ConfigureMemory(Configuration const& config_v) {
	m_Memory.ConfigureMemory(config_v);
	m_Memory.ConfigureBiosROM(config_v);
}

Machine::~Machine() 
{}

auto Machine::Start() -> void
{
	s_log.StartMachine();
	m_VideoDevice.Start();
	m_ProcessorExit = m_Processor.Start();
}

auto Machine::Stop() -> void
{
	s_log.StopMachine();
	m_Processor.Stop();
	if (m_ProcessorExit.valid()) {
		m_ProcessorExit.wait();
	}
	m_VideoDevice.Stop();
}

auto Machine::Reset() -> void
{
	SuspendAllProcessors();
	m_Processor.Reset();
	m_Partition.Reset();
	m_Debugger.Reset();
	m_LegacyPic.Reset();
	ResumeAllProcessors();
}

auto Machine::RunMain() -> void
{
	using utils::logger;
	using namespace std::chrono_literals;	
	using namespace win32;
	using namespace win32::regs;
	
	if (!m_ProcessorExit.valid() || std::future_status::ready != m_ProcessorExit.wait_for(0s))
		return;
	auto const [status_v, context_v] = m_ProcessorExit.get();
	std::exchange(m_ProcessorExit, {});
	if (status_v != ERROR_SUCCESS) {
		throw win32::error(status_v);
	}	
	switch (context_v.ExitReason)
	{
	case WHvRunVpExitReasonCanceled:				
		s_log.VCpuExited(m_Processor.GetIndex(), context_v);
		return;
	default: // unexpected exit reason
		s_log.VCpuExited(m_Processor.GetIndex(), context_v);
		throw std::runtime_error(__func__);		
	}
}

auto Machine::SetIRQ(uint16_t state_v) -> void
{
	s_log.IRQState(m_Processor.GetIndex(), state_v);
	m_LegacyPic.SetIRQ(state_v);
}

auto Machine::ConfigurePartition(Configuration const&) -> void
{
	using namespace win32;

	auto const synic_features_v = WHvCapabilities::Get
		<WHV_SYNTHETIC_PROCESSOR_FEATURES_BANKS>
		(WHvCapabilityCodeSyntheticProcessorFeaturesBanks);

	m_Partition = win32::WHvPartition::Create(1u, {		
		{ WHvPartitionPropertyCodeExceptionExitBitmap, { 
			.ExceptionExitBitmap 
			= (1u << WHvX64ExceptionTypeDoubleFaultAbort)
			| (1u << WHvX64ExceptionTypeInvalidOpcodeFault)			
			| (1u << WHvX64ExceptionTypeGeneralProtectionFault)
			| (1u << WHvX64ExceptionTypeDebugTrapOrFault)  
			} },
		{ WHvPartitionPropertyCodeX64MsrExitBitmap, {.X64MsrExitBitmap = {.UnhandledMsrs = 1 } } },
		{ WHvPartitionPropertyCodeExtendedVmExits, { .ExtendedVmExits = { .X64MsrExit = 1u, .ExceptionExit = 1u, .HypercallExit = 1u } } },
		{ WHvPartitionPropertyCodeProcessorCount, { .ProcessorCount = 1u } },		
		//{ WHvPartitionPropertyCodeSyntheticProcessorFeaturesBanks, { .SyntheticProcessorFeaturesBanks = synic_features_v } },
	  //{ WHvPartitionPropertyCodeLocalApicEmulationMode, { .LocalApicEmulationMode = WHvX64LocalApicEmulationModeXApic } },
		{ WHvPartitionPropertyCodeLocalApicEmulationMode, {.LocalApicEmulationMode = WHvX64LocalApicEmulationModeNone } },
		{ WHvPartitionPropertyCodeProcessorFeatures, { .ProcessorFeatures = WHvCapabilities::Get<WHV_PROCESSOR_FEATURES>(WHvCapabilityCodeProcessorFeatures) } }
	});
}

auto Machine::SuspendAllProcessors() -> void
{
	m_Processor.Suspend();
}

auto Machine::ResumeAllProcessors() -> void
{
	m_Processor.Resume();
}


auto Machine::IoPortAccess(Processor const& vcpu_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t
{
#define MAP_RANGE_REL(lhs_v, rhs_v, dst_v) if(port_v >= lhs_v && port_v <= rhs_v) \
	return dst_v.IoPortAccess(vcpu_v, is_write_v, port_v - lhs_v, data_v)
#define MAP_RANGE_ABS(lhs_v, rhs_v, dst_v) if(port_v >= lhs_v && port_v <= rhs_v) \
	return dst_v.IoPortAccess(vcpu_v, is_write_v, port_v, data_v)

	MAP_RANGE_REL(0x020u, 0x021u, m_LegacyPic.Master());
	MAP_RANGE_REL(0x0A0u, 0x0A1u, m_LegacyPic.Slave());
	MAP_RANGE_REL(0x0E8u, 0x0EAu, m_Debugger);	
	MAP_RANGE_ABS(0x3B0u, 0x3DFu, m_VideoDevice);

#undef MAP_RANGE_REL
#undef MAP_RANGE_ABS
	__debugbreak();
	return 0;
}


auto Machine::Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> int32_t
{		
	switch (hypercall_v.Major)
	{	  
	case 0x00: return m_VideoDevice.Hypercall(vcpu_v, hypercall_v);		
	case 0xFF: return m_Debugger.Hypercall(vcpu_v, hypercall_v);
	default: break;
	}
	
	return ERROR_ACCESS_DENIED;
}

auto Machine::MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v) -> int32_t
{	
	using std::ranges::fill;
/*
	if (((~0xFFFull)&(physaddr_v + data_v.size())) != ((~0xFFFull)&physaddr_v))
	{
		uint64_t offset_v{ 0x1000ull - (physaddr_v&0xFFFull) };
		int32_t status_v{ ERROR_SUCCESS };
		status_v = MemoryAccess(vcpu_v, is_write_v, physaddr_v, data_v.first(offset_v));
		if (status_v != ERROR_SUCCESS) return status_v;
		return MemoryAccess(vcpu_v, is_write_v, physaddr_v+offset_v, data_v.subspan(offset_v));
	}
*/
	if (physaddr_v >= 0xA0000u && physaddr_v <= 0xBFFFFu) {
		return m_VideoDevice.MemoryAccess(vcpu_v, is_write_v, physaddr_v, data_v);
	}

	if (physaddr_v >= 0xC0000u && physaddr_v <= 0xFFFFFu) {
		if (!is_write_v) fill(data_v, std::byte{0xff});		
		return ERROR_SUCCESS;
	}

	return m_Memory.MemoryAccess(vcpu_v, is_write_v, physaddr_v, data_v);
}
