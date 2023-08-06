#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/workqueue.hpp>

#include <core/configuration.hpp>
#include <core/memory.hpp>
#include <core/processor.hpp>
#include <core/cputhread.hpp>
#include <core/debugger.hpp>

#include <utils/span.hpp>

#include <shared_mutex>
#include <memory>
#include <mutex>
#include <list>


namespace core
{
	struct Machine
	{
		Machine(Configuration const&);
		~Machine();

		auto Start() -> void;
		auto Stop() -> void;
		auto Reset() -> void;
		auto RunMain() -> void;

		auto Interrupt(std::uint8_t vector_v) -> void;
	
	protected:
		friend Processor;
		friend CpuThread;

		auto Partition() const -> win32::WHvPartition const& { return m_Partition; }
		auto Emulator() const -> win32::WHvEmulator const& { return m_Emulator;  }
		auto Processor() const -> Processor const& { return m_Processor; }

		auto IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;		
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

	private:
		std::shared_mutex m_StateMutex;
		win32::WHvEmulator m_Emulator;
		win32::WHvPartition m_Partition;
		std::list<Memory> m_Memory;
		core::Processor m_Processor;
		core::CpuThread m_CpuThread;
		core::Debugger m_Debugger;
	};

}
