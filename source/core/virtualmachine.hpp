#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvprocessor.hpp>

#include <core/virtualmemory.hpp>
#include <core/configuration.hpp>
#include <core/processorthread.hpp>

#include <shared_mutex>
#include <memory>
#include <mutex>
#include <list>


namespace core
{
	struct VirtualMachine
	{
		VirtualMachine(Configuration const&);
		~VirtualMachine();

		auto Start     () -> void;
		auto Stop      () -> void;
		auto Reset     () -> void;

		auto RunMain	 () -> void;
	
	protected:
		friend ProcessorThread;

		auto Partition ()	const -> win32::WHvPartition const& { return m_Partition; }
		auto Emulator  ()	const -> win32::WHvEmulator const&  { return m_Emulator;  }

		auto ProcessorExit(WHV_RUN_VP_EXIT_CONTEXT& exit_v, win32::WHvProcessor const& processor_v) -> void;
		
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

	private:

		std::shared_mutex m_StateMutex;
		win32::WHvPartition m_Partition;
		win32::WHvEmulator m_Emulator;
		std::list<VirtualMemory> m_Memory;
		ProcessorThread m_ProcessorThread;
	};

}
