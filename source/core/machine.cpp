#include <win32/whvcapabilities.hpp>

#include <core/machine.hpp>

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
{
	ConfigurePartition(config_v);
	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);	
	WIN32_ERROR_ASSERT(m_LegacyPic.Initialize());
	WIN32_ERROR_ASSERT(m_LegacyVideo.Initialize());
}

Machine::~Machine() 
{}

auto Machine::Start() -> void
{
	using utils::logger;
	logger::info(logger::deflog, "Starting machine...");
	m_ProcessorExit = m_Processor.Start();
}

auto Machine::Stop() -> void
{
	using utils::logger;
	logger::info(logger::deflog, "Stopping machine...");
	m_Processor.Stop();
	if (m_ProcessorExit.valid()) {
		m_ProcessorExit.wait();
	}
}

auto Machine::Reset() -> void
{
	Stop();
	m_Processor.Reset();
	m_Partition.Reset();
	m_Debugger.Reset();
	Start();
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
		logger::info(logger::deflog, "CPU[{}] stopped, reason=Cancel", m_Processor.GetIndex());
		return;
	default:
		// Unexpected cpu exit
		throw std::runtime_error(std::format(
			"CPU[{}] exited unexpectedly with reason={:#x}, rebooting...", 
			m_Processor.GetIndex(), (std::uint32_t)context_v.ExitReason));
		return Reset();
	}
}

auto Machine::Render() -> 
	std::tuple<LegacyVideo::buffer_type, std::chrono::microseconds>
{
	return m_LegacyVideo.Render();
}

auto Machine::SetIRQ(std::uint16_t state_v) -> void
{
	using utils::logger;
	// Conflicting IRQs are not changed
	logger::info(logger::deflog, "CPU[{}] raised IRQ [{}]", 
		m_Processor.GetIndex(), utils::bitset_to_string(state_v));
	m_LegacyPic.SetIRQ(state_v);
}

auto Machine::ConfigurePartition(Configuration const&) -> void
{
	using namespace win32;

	auto const synic_features_v = WHvCapabilities::Get
		<WHV_SYNTHETIC_PROCESSOR_FEATURES_BANKS>
		(WHvCapabilityCodeSyntheticProcessorFeaturesBanks);

	m_Partition = win32::WHvPartition::Create(1u, {		
		{ WHvPartitionPropertyCodeExceptionExitBitmap, { .ExceptionExitBitmap 
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

auto Machine::ConfigureMemory(Configuration const&) -> void
{
	WIN32_ERROR_ASSERT(m_Partition.Reset());	
	std::uint64_t memory_size_v = 16_MiB;
	if (memory_size_v > 0u) {
		auto basemem_size_v = std::min(memory_size_v, 640_KiB);
		memory_size_v -= basemem_size_v;
		assert(basemem_size_v + 384_KiB <= 1_MiB);
		m_Memory.emplace_back(m_Partition, 0, basemem_size_v, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		auto extmem_size_v = std::min(memory_size_v, 14_MiB);
		memory_size_v -= extmem_size_v;
		assert(extmem_size_v + 2_MiB <= 16_MiB);
		m_Memory.emplace_back(m_Partition, 1_MiB, extmem_size_v, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		auto paemem_size_v = std::min(memory_size_v, 3056_MiB);
		memory_size_v -= paemem_size_v;
		assert (paemem_size_v + 16_MiB <= 3072_MiB);
		m_Memory.emplace_back(m_Partition, 16_MiB, paemem_size_v, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		m_Memory.emplace_back(m_Partition, 4096_MiB, memory_size_v, kAccessMemory);
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

	path_v = utils::path_substitute(path_v);
	if (!std::filesystem::exists(path_v)) {
		throw std::runtime_error("BIOS file not found");
	}
	auto size_v = std::filesystem::file_size(path_v);
	if (size_v < 4_KiB || size_v > 256_KiB) {
		throw std::runtime_error("BIOS size should be between 4KiB and 256KiB");
	}

	size_v = (size_v + kPageSize - 1u) & ~(kPageSize - 1u);
	auto addr_v = 1_MiB - size_v;
	m_Memory.emplace_back(m_Partition, addr_v, size_v, kAccessReadOnly);
	m_Memory.back().Load(path_v);
}

auto Machine::IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
#define MAP_RANGE(lhs, rhs, target) if(port_v>=lhs&&port_v<=rhs) \
	return target.IoPortAccess(vcpu_v, is_write_v, port_v-lhs, data_v)

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
