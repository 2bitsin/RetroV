#include <core/virtualmachinebase.hpp>

using core::VirtualMachineBase;

VirtualMachineBase::VirtualMachineBase()
	: m_Partition{ WHvPartition::Create() }
	, m_Emulator{ WHvEmulator::Create() }
	, m_MemoryMap{ 
{

}