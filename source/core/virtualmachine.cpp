#include <core/virtualmachine.hpp>

using core::VirtualMachine;

VirtualMachine::VirtualMachine()
	: m_Partition{ win32::WHvPartition::Create() }
	, m_Emulator{ win32::WHvEmulator::Create() }
	, m_MemoryMap{ *this }
{}

VirtualMachine::~VirtualMachine() 
{}
