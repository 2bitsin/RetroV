#include <win32/whvcapabilities.hpp>
#include <core/virtualmachine.hpp>
#include <utils/literals.hpp>
#include <utils/paths.hpp>

using namespace size_literals;

using core::VirtualMachine;

VirtualMachine::VirtualMachine(Configuration const& config_v)
	: m_Partition { 
			{ WHvPartitionPropertyCodeProcessorCount, { 
				.ProcessorCount = 1u }},
		  { WHvPartitionPropertyCodeExtendedVmExits, { 
				.ExtendedVmExits = { 
					.HypercallExit = 1u }}}
		}
	, m_Emulator  {  }	
{
	ConfigureBiosROM(config_v);
	ConfigureMemory(config_v);
}

VirtualMachine::~VirtualMachine() 
{}

auto VirtualMachine::ConfigureMemory(Configuration const&) -> void
{
	std::uint64_t memory_size_v = 16_MiB;

	if (memory_size_v > 0u) {
		auto basemem_size_v = std::max(memory_size_v, 640_KiB);
		memory_size_v -= basemem_size_v;
		assert(basemem_size_v + 384_KiB <= 1_MiB);
		m_Memory.emplace_back(m_Partition, 0 / kPageSize, basemem_size_v / kPageSize, kAccessMemory);
	}

	if (memory_size_v > 0u) {
		auto extmem_size_v = std::max(memory_size_v, 14_MiB);
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

auto VirtualMachine::ConfigureBiosROM(Configuration const&) -> void
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
}
