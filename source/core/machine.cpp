#include <win32/whvcapabilities.hpp>
#include <core/machine.hpp>
#include <utils/literals.hpp>
#include <utils/logger.hpp>
#include <utils/paths.hpp>
#include <utils/span.hpp>


using namespace size_literals;

using core::Machine;

Machine::Machine(Configuration const& config_v)	
	: m_Partition { nullptr }
	, m_Processor { *this, 0u }
	, m_Debugger  { *this }
{
	ConfigurePartition(config_v);
	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);
}

Machine::~Machine() 
{}

auto Machine::Start() -> void
{
	using utils::logger;
	logger::info(logger::deflog, "Starting machine...");
	m_ProcessorExit = m_Processor.RunAsync();
}

auto Machine::Stop() -> void
{
	using utils::logger;
	logger::info(logger::deflog, "Stopping machine...");
	m_Processor.CancelAsync();
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
		logger::info(logger::deflog, 
			"CPU[{}] exited, reason=Cancelled", 
			m_Processor.GetIndex(),
			(std::uint32_t)context_v.ExitReason);
		return;
	default:
		logger::error(logger::deflog, 
			"CPU[{}] exited unexpectedly with reason={:#x}, rebooting...", 
			m_Processor.GetIndex(),
			(std::uint32_t)context_v.ExitReason);
		return Reset();
	}
}

auto Machine::RaiseIRQ(std::uint8_t vector_v) -> void
{
	using utils::logger;
	logger::info(logger::deflog, "CPU[{}] raised IRQ[{}]", m_Processor.GetIndex(), vector_v);
	WIN32_ERROR_ASSERT(m_Processor.RequestIRQ(vector_v));
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

auto Machine::ConfigureMemory(Configuration const&) -> void
{
	WIN32_ERROR_ASSERT(m_Partition.Reset());	
	std::uint64_t memory_size_v = 16_MiB;
	if (memory_size_v > 0u) {
		auto basemem_size_v = std::min(memory_size_v, 640_KiB);
		memory_size_v -= basemem_size_v;
		assert(basemem_size_v + 384_KiB <= 1_MiB);
		m_Memory.emplace_back(m_Partition, 0 / kPageSize, basemem_size_v / kPageSize, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		auto extmem_size_v = std::min(memory_size_v, 14_MiB);
		memory_size_v -= extmem_size_v;
		assert(extmem_size_v + 2_MiB <= 16_MiB);
		m_Memory.emplace_back(m_Partition, 1_MiB/kPageSize, extmem_size_v/kPageSize, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		auto paemem_size_v = std::min(memory_size_v, 3056_MiB);
		memory_size_v -= paemem_size_v;
		assert (paemem_size_v + 16_MiB <= 3072_MiB);
		m_Memory.emplace_back(m_Partition, 16_MiB/kPageSize, paemem_size_v/kPageSize, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		m_Memory.emplace_back(m_Partition, 4096_MiB/kPageSize, memory_size_v/kPageSize, kAccessMemory);
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
	m_Memory.emplace_back(m_Partition, addr_v / kPageSize, size_v / kPageSize, kAccessReadOnly);
	m_Memory.back().Load(path_v);
}

auto Machine::IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t
{
	using utils::logger;
	std::uint32_t what_v{ 0 };
	std::int32_t result_v{ };
	switch (port_v) {
	case 0xe8: 
	case 0xe9: return m_Debugger.IoPortAccess(is_write_v, port_v, data_v);
	case 0xea: 
		{
			WHV_REGISTER_VALUE reg_v { };
			if (is_write_v) {
				logger::info(logger::deflog, "CPU[{}] flat real mode hack enabled!", m_Processor.GetIndex());
				reg_v = m_Processor.GetRegister(WHvX64RegisterDs);
				reg_v.Segment.Limit = 0xFFFFFFFFu;
				reg_v.Segment.Base = 0u;
				reg_v.Segment.Attributes = 0xCF93u;
				m_Processor.SetRegister(WHvX64RegisterDs, reg_v);

				reg_v = m_Processor.GetRegister(WHvX64RegisterEs);
				reg_v.Segment.Limit = 0xFFFFFFFFu;
				reg_v.Segment.Base = 0u;
				reg_v.Segment.Attributes = 0xCF93u;
				m_Processor.SetRegister(WHvX64RegisterEs, reg_v);

				reg_v = m_Processor.GetRegister(WHvX64RegisterFs);
				reg_v.Segment.Limit = 0xFFFFFFFFu;
				reg_v.Segment.Base = 0u;
				reg_v.Segment.Attributes = 0xCF93u;
				m_Processor.SetRegister(WHvX64RegisterFs, reg_v);

				reg_v = m_Processor.GetRegister(WHvX64RegisterGs);
				reg_v.Segment.Limit = 0xFFFFFFFFu;
				reg_v.Segment.Base = 0u;
				reg_v.Segment.Attributes = 0xCF93u;
				m_Processor.SetRegister(WHvX64RegisterGs, reg_v);
			}
			break;
		}
	default:
		__debugbreak();
	}
	return 0;
}

auto Machine::MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	__debugbreak();
	return std::int32_t();
}
