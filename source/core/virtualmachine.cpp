#include <core/virtualmachine.hpp>
#include <utils/literals.hpp>

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
	std::uint64_t memory_size_v = 16_MiB;



}

VirtualMachine::~VirtualMachine() 
{}
