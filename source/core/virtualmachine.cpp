#include <core/virtualmachine.hpp>

using core::VirtualMachine;

static inline auto ConfigurationGetProcessorCount(core::Configuration const& config_v) -> std::uint64_t {
	return config_v.GetPropertyUint64("processor.count");
}

VirtualMachine::VirtualMachine(Configuration const& config_v)
	: m_Partition { 
			{ WHvPartitionPropertyCodeProcessorCount, { 
				.ProcessorCount = (std::uint32_t)ConfigurationGetProcessorCount(config_v) }},
		  { WHvPartitionPropertyCodeExtendedVmExits, { 
				.ExtendedVmExits = { 
					.HypercallExit = 1u }}}
		}
	, m_Emulator  {  }
	, m_Processor { m_Partition, 0u }	 
{}

VirtualMachine::~VirtualMachine() 
{}
