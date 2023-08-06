#include <win32/whvcapabilities.hpp>
#include <core/machine.hpp>
#include <utils/literals.hpp>
#include <utils/paths.hpp>
#include <utils/span.hpp>


using namespace size_literals;

using core::Machine;

Machine::Machine(Configuration const& config_v)
	
	: m_Partition { nullptr }
	, m_Processor { *this, 0u }
	, m_Debugger  { }
	, m_CpuThread { }
{
	ConfigurePartition(config_v);
	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);
}

Machine::~Machine() 
{}

auto Machine::Start() -> void
{
	m_CpuThread.Start(m_Processor);
}

auto Machine::Stop() -> void
{
	m_CpuThread.Stop();
}

auto Machine::Reset() -> void
{
	m_CpuThread.Stop();
	m_Processor.Reset();
	m_Partition.Reset();
	m_Debugger.Reset();
	m_CpuThread.Start(m_Processor);
}

auto Machine::RunMain() -> void
{
}

auto Machine::Interrupt(std::uint8_t vector_v) -> void
{
	WIN32_ERROR_ASSERT(m_Processor.RequestInterrupt(vector_v));
}

auto Machine::ConfigurePartition(Configuration const&) -> void
{
	
	m_Partition = win32::WHvPartition::Create(1u, {
		{ WHvPartitionPropertyCodeExtendedVmExits, { .ExtendedVmExits = { .HypercallExit = 1u } } }	  
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
	switch (port_v) {
	case 0xe9: return m_Debugger.IoPortAccess(is_write_v, port_v, data_v);
	}
	return 0;
}

auto Machine::MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t
{
	__debugbreak();
	return std::int32_t();
}
