#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>

#include <core/memorymap.hpp>

namespace core
{
	struct VirtualMachineBase
	{
		VirtualMachineBase();

	private:
		win32::WHvPartition m_Partition;
		win32::WHvEmulator m_Emulator;
		core::MemoryMap m_MemoryMap;
	};

}
