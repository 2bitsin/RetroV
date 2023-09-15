#include <system_error>
#include <filesystem>
#include <algorithm>
#include <ranges>


#include <win32/whvcapabilities.hpp>

#include <core/machine.hpp>

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
	, m_LegacyPic		{ *this, 0u }
	, m_Debugger		{ *this }
	, m_VideoDevice { *this }
	, m_Display			{ *this }
{
	ConfigurePartition(config_v);
	ConfigureMemory(config_v);	
	ConfigureBiosROM(config_v);
	m_LegacyPic.Initialize();
	m_Display.Initialize();
	m_VideoDevice.Initialize(config_v);
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

auto Machine::SetIRQ(std::uint16_t state_v) -> void
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

auto Machine::ConfigureMemory(Configuration const& config_v) -> void
{
	static constexpr utils::region64_type ram_map_s [] = 
	{
		{ utils::from_range, 0x00000000u, 0x000A0000u }, // Coventional memory
	//{ utils::from_range, 0x000A0000u, 0x00100000u }, // ROM Area
		{ utils::from_range, 0x00100000u, 0x00200000u }, // A20 memory mirror 
		{ utils::from_range, 0x00200000u, 0x00F00000u }, // Extended memory under 15MiB
	//{ utils::from_range, 0x00F00000u, 0x01000000u }, // ISA memory hole
		{ utils::from_range, 0x01000000u, 0xC0000000u }, // Extended memory over 15MiB
	//{ utils::from_range, 0xC0000000u, 0xFEE00000u }, // PCI memory hole
	//{ utils::from_range, 0xFEE00000u, 0xFEE01000u }, // Local APIC
	//{ utils::from_range, 0xFEE01000u, 0xFFE00000u }, // Unused ?
	//{ utils::from_range, 0xFFE00000u, 0xFFEFFFFFu }, // High part of BIOS ROM

		// Remaining
	  { utils::from_range, 
		  0x0000000100000000u, 
		  0xFFFFFFFFFFFFFFFFu },
	};

	static constexpr utils::region64_type empty_regions_s [] = {
		{ utils::from_range, 0x000A0000u, 0x00100000u }
	};

	static constexpr auto default_page_s = utils::make_filled_array<std::uint8_t, kPageSize>(0xFFu);

	WIN32_ERROR_ASSERT(m_Partition.Reset());	

	m_MainMemory = win32::VirtualAlloc_s(
		config_v.GetPropertyUint64("memory.size.megabytes") * 1_MiB, 
		win32::execute_read_write);

	auto& partition_v = GetPartition();
	auto memory_v = std::span(m_MainMemory);
	for (auto&& window_v : ram_map_s) 
	{
		if (memory_v.empty()) break;		
		auto slice_v = utils::take_slice(memory_v, window_v.size());
		m_MappedRanges.emplace_back(partition_v, window_v, kAccessMemory, slice_v);
	}
	
	for (auto&& region_v : empty_regions_s)
	for (auto curr_page_v = region_v.begin(); 
		curr_page_v < region_v.end(); 
		curr_page_v += default_page_s.size()) 
	{
		partition_v.MapGpaRange(default_page_s.data(), curr_page_v, 
			default_page_s.size(), kAccessReadOnly);
	}

}

auto Machine::ConfigureBiosROM(Configuration const& config_v) -> void
{
	
	auto const path_v = config_v.GetPropertyString("rom.path.system");	
	static constexpr auto const region_lo = RomImage::region_type{ utils::size_invert, 1_MiB, 192_KiB };
	static constexpr auto const check_lo = RomImage::validate{ 4_KiB, 1u, region_lo.size() / 4_KiB };
	static constexpr auto const region_hi = RomImage::region_type{ utils::size_invert, 4_GiB, 16_MiB };
	static constexpr auto const check_hi = RomImage::validate{ 4_KiB, 1u, region_hi.size() / 4_KiB };
	static constexpr auto const align_v = RomImage::kTopAligned;
	
	m_MappedRoms.emplace_back(m_Partition, check_lo, path_v, region_lo, align_v);
	m_MappedRoms.emplace_back(m_Partition, check_hi, path_v, region_hi, align_v);
}

auto Machine::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
#define MAP_RANGE(lhs_v, rhs_v, dst_v) if(port_v >= lhs_v && port_v <= rhs_v) \
	return dst_v.IoPortAccess(vcpu_v, is_write_v, port_v - lhs_v, data_v)

	MAP_RANGE(0x020u, 0x021u, m_LegacyPic.Master());
	MAP_RANGE(0x0A0u, 0x0A1u, m_LegacyPic.Slave());
	MAP_RANGE(0x0E8u, 0x0EAu, m_Debugger);	
	MAP_RANGE(0x3B0u, 0x3DFu, m_VideoDevice);

#undef MAP_RANGE
	__debugbreak();
	return 0;
}


auto Machine::Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t
{		
	switch (hypercall_v.Major)
	{	  
	case 0x00: return m_VideoDevice.Hypercall(vcpu_v, hypercall_v);		
	case 0xFF: return m_Debugger.Hypercall(vcpu_v, hypercall_v);
	default: break;
	}
	
	return ERROR_ACCESS_DENIED;
}

auto Machine::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t physaddr_v, utils::limited_span<std::byte, 16u> data_v) -> std::int32_t
{	
	using std::ranges::fill;

	if (physaddr_v >= 0xA0000u && physaddr_v <= 0xBFFFFu) {
		return m_VideoDevice.MemoryAccess(vcpu_v, is_write_v, physaddr_v, data_v);
	}

	if (physaddr_v >= 0xC0000u && physaddr_v <= 0xFFFFFu) {
		if (!is_write_v) fill(data_v, std::byte{0xff});		
		return ERROR_SUCCESS;
	}

	return ERROR_ACCESS_DENIED;
}
