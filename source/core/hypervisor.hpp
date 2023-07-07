#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <mutex>

#include <utils/span.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>

#include <core/config.hpp>
#include <core/partition.hpp>
#include <core/scheduler.hpp>

#include <core/debug/debugger.hpp>
#include <core/mem/block.hpp>
#include <core/mem/pool.hpp>
#include <core/mem/manager.hpp>
#include <core/processor/registers.hpp>
#include <core/processor.hpp>
#include <core/eventbroker.hpp>

namespace core
{ 
	struct Hypervisor
	{
		
		/*********************************
		 *  Constructors and destructors
		 *********************************/

		Hypervisor(Hypervisor const&) = delete;
		auto operator = (Hypervisor const&)->Hypervisor & = delete;
		Hypervisor(Hypervisor&&) = delete;
		auto operator = (Hypervisor&&)->Hypervisor & = delete;

		Hypervisor (Config const&);
	  ~Hypervisor ();

		auto GetParitionHandle() -> WHV_PARTITION_HANDLE;		
		auto GetScheduler() -> Scheduler&;
		auto GetPartition() -> Partition&;
		auto GetEventBroker() -> EventBroker&;
		auto GetMemPool() -> mem::Pool&;
		auto GetMemManager() -> mem::Manager&;
		auto GetProcessor(std::uint32_t index_v) -> Processor&;

		/**********************************
		 *  Processor configuration methods
		 **********************************/

		auto PowerOn() -> void;
		auto Shutdown() -> void;

	protected:
		friend struct Config;

		/************************
		 *  Misc internal methods
		 ************************/
		auto InitializePartition() -> void;
		auto InitializeProcessor(std::uint32_t index_v) -> void;

		/****************
		 * Internal state
		 ****************/
	private:
		Scheduler m_Scheduler;
		Partition m_Partition;
		EventBroker m_EventBroker;
		mem::Pool m_MemPool;
		mem::Manager m_MemManager;
		std::vector<Processor> m_Processors;		
		debug::Debugger m_Debugger;
	};
}