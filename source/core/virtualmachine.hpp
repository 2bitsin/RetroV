#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvprocessor.hpp>

#include <core/configuration.hpp>

namespace core
{
	struct VirtualMachine
	{
		VirtualMachine(Configuration const&);
		~VirtualMachine();

		auto Start     () -> void;
		auto Suspend   () -> void;
		auto Resume    () -> void;
		auto Stop      () -> void;
		auto Reset     () -> void;
	
	protected:
		auto Partition ()	const -> win32::WHvPartition const& { return m_Partition; }
		auto Emulator  ()	const -> win32::WHvEmulator  const& { return m_Emulator;  }		
	
	private:
		win32::WHvPartition m_Partition;
		win32::WHvEmulator m_Emulator;
	};

}
