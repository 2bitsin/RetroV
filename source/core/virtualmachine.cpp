#include <core/virtualmachine.hpp>

using core::VirtualMachine;

VirtualMachine::VirtualMachine()
	: m_Partition{ WHvPartition::Create() }
	, m_Emulator{ WHvEmulator::Create() }
	, m_MemoryMap{ *this }
{}

VirtualMachine::~VirtualMachine() 
{}
