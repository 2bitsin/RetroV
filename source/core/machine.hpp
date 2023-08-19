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
#include <core/localapic.hpp>
#include <core/pic8259.hpp>
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

		auto RaiseIRQ(std::uint8_t vector_v) -> void;
		auto GetProcessor(std::uint32_t vcpuindex_v) -> Processor& { (void)vcpuindex_v; return m_Processor; }
		auto GetLocalApic(std::uint32_t vcpuindex_v) -> LocalApic& { (void)vcpuindex_v; return m_LocalApic; }
		auto GetPartition() -> win32::WHvPartition& { return m_Partition; }

	protected:
		friend Processor;
		friend Debugger;


		auto IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;		
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

	private:		
		win32::WHvPartition m_Partition;
		std::list<Memory> m_Memory;
		core::Processor m_Processor;
		core::Processor::exit_future_type m_ProcessorExit;
		core::LocalApic m_LocalApic;
		core::Pic8259 m_Pic8259;
		core::Debugger m_Debugger;
	};

}
