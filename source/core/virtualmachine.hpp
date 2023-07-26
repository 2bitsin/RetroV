#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvprocessor.hpp>

#include <core/virtualmemory.hpp>
#include <core/virtualprocessor.hpp>
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
		friend VirtualProcessor;

		auto Partition ()	const -> win32::WHvPartition const& { return m_Partition; }
		auto Emulator  ()	const -> win32::WHvEmulator const&  { return m_Emulator;  }

		auto ProcessorExit(WHV_RUN_VP_EXIT_CONTEXT& exit_v, std::uint32_t vcpuindex_v) -> void;		
		auto IoPortAccess(bool is_write_v, std::uint16_t port_v, std::uint8_t size_v, utils::bytes<4u>& data_v) -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, std::uint8_t size_v, utils::bytes<8u>& data_v) -> std::int32_t;
		
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

	private:

		std::shared_mutex m_StateMutex;
		win32::WHvPartition m_Partition;
		win32::WHvEmulator m_Emulator;
		std::list<VirtualMemory> m_Memory;
		VirtualProcessor m_Processor;
		ProcessorThread m_ProcessorThread;
	};

}
