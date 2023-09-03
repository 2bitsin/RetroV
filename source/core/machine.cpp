#include <system_error>
#include <filesystem>

#include <win32/whvcapabilities.hpp>

#include <core/machine.hpp>

#include <utils/region.hpp>
#include <utils/algorithm.hpp>
#include <utils/literals.hpp>
#include <utils/logger.hpp>
#include <utils/paths.hpp>
#include <utils/span.hpp>

#include <SDL2/SDL.h>

#undef main

using namespace size_literals;

using core::Machine;

Machine::Machine(Configuration const& config_v)	
	: m_Partition		{ nullptr }
	, m_Processor		{ *this, 0u }
	, m_LegacyPic		{ *this, 0u }
	, m_Debugger		{ *this }
	, m_LegacyVideo { *this }
	, m_Display			{ *this }
{
	ConfigurePartition(config_v);
	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);	
	m_LegacyPic.Initialize();
	m_LegacyVideo.Initialize();
	m_Display.Initialize();
}

Machine::~Machine() 
{}

auto Machine::Start() -> void
{
	s_log.StartMachine();
	m_LegacyVideo.Start();
	m_ProcessorExit = m_Processor.Start();
}

auto Machine::Stop() -> void
{
	s_log.StopMachine();
	m_Processor.Stop();
	if (m_ProcessorExit.valid()) {
		m_ProcessorExit.wait();
	}
	m_LegacyVideo.Stop();
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
			= (1u << WHvX64ExceptionTypeGeneralProtectionFault)
			| (1u << WHvX64ExceptionTypeDoubleFaultAbort)
			| (1u << WHvX64ExceptionTypeInvalidOpcodeFault)
			| (1u << WHvX64ExceptionTypeDebugTrapOrFault)
 		} },
		{ WHvPartitionPropertyCodeX64MsrExitBitmap, {.X64MsrExitBitmap = {.UnhandledMsrs = 1 } } },
		{ WHvPartitionPropertyCodeExtendedVmExits, { .ExtendedVmExits = { .X64MsrExit = 1u, .ExceptionExit = 1u, .HypercallExit = 1u } } },
		{ WHvPartitionPropertyCodeProcessorCount, { .ProcessorCount = 1u } },		
		{ WHvPartitionPropertyCodeSyntheticProcessorFeaturesBanks, { .SyntheticProcessorFeaturesBanks = synic_features_v } },
	  { WHvPartitionPropertyCodeLocalApicEmulationMode, { .LocalApicEmulationMode = WHvX64LocalApicEmulationModeXApic } },
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
	static constexpr utils::region_64_t ram_map_s [] = 
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

	WIN32_ERROR_ASSERT(m_Partition.Reset());	

	m_MainMemory = win32::VirtualAlloc_s(
		config_v.GetPropertyUint64("memory.size.megabytes") * 1_MiB, 
		win32::execute_read_write);

	auto memory_v = std::span(m_MainMemory);
	for (auto&& window_v : ram_map_s) 
	{
		if (memory_v.empty()) break;		
		auto slice_v = utils::take_slice(memory_v, window_v.size());
		s_log.MapGpaRange(slice_v.data(), window_v.base(), slice_v.size(), kAccessMemory);
		m_MappedRanges.emplace_back(GetPartition(), window_v, kAccessMemory, slice_v);
	}
}

auto Machine::ConfigureBiosROM(Configuration const&) -> void
{
	std::filesystem::path path_v;
	
	if (win32::WHvCapabilities::IsVendorAMD()) {
		path_v = "@base/ROMs/BiosAMD.bin";
	} else if (win32::WHvCapabilities::IsVendorIntel()) {
		path_v = "@base/ROMs/BiosIntel.bin";
	} else {
		throw std::runtime_error("Unsupported CPU vendor");
	}

	path_v = utils::build_path(path_v);

	if (!std::filesystem::exists(path_v)) {
		throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory), path_v.string());
	}

	auto size_v = std::filesystem::file_size(path_v);

	if (size_v < 4_KiB || size_v > 4_MiB) {
		throw std::runtime_error("BIOS size should be between 4KiB and 4MiB");
	}

	if (utils::round_ceil(size_v, 4_KiB) != size_v) {
		throw std::runtime_error("BIOS size should be a multiple of 4KiB");
	}

	auto const size_lo_v = std::min(size_v, 256_KiB);
	utils::region_64_t region_lo_v{ 0x0000000000100000u - size_lo_v, size_lo_v };

	auto const size_hi_v = std::min(size_v, 32_MiB);
	utils::region_64_t region_hi_v{ 0x0000000100000000u - size_hi_v, size_hi_v };

	m_MappedRoms.emplace_back(path_v, utils::region_64_t{0, size_v}, win32::open_existing, win32::read_only);
	auto const& bios_v = m_MappedRoms.back();
	
	s_log.MapGpaRangeFromFile(bios_v.Data().data(), region_lo_v.base(), bios_v.Data().size(), path_v, 0, size_v);
	m_MappedRanges.emplace_back(GetPartition(), region_lo_v, kAccessReadOnly, bios_v.Data());

	s_log.MapGpaRangeFromFile(bios_v.Data().data(), region_hi_v.base(), bios_v.Data().size(), path_v, 0, size_v);
	m_MappedRanges.emplace_back(GetPartition(), region_hi_v, kAccessReadOnly, bios_v.Data());
	
}

auto Machine::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
#define MAP_RANGE(lhs_v, rhs_v, dst_v) if(port_v >= lhs_v && port_v <= rhs_v) \
	return dst_v.IoPortAccess(vcpu_v, is_write_v, port_v - lhs_v, data_v)

	MAP_RANGE(0x020u, 0x021u, m_LegacyPic.Master());
	MAP_RANGE(0x0A0u, 0x0A1u, m_LegacyPic.Slave());
	MAP_RANGE(0x0E8u, 0x0EAu, m_Debugger);	

#undef MAP_RANGE
	__debugbreak();
	return 0;
}

auto Machine::MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{	
	__debugbreak();
	return ERROR_ACCESS_DENIED;
}
