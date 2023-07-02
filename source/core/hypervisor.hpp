#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <mutex>

#include <utils/span_as.hpp>
#include <win32/error.hpp>
#include <win32/winhvpx.hpp>
#include <core/config.hpp>
#include <core/vmc/manager.hpp>
#include <core/mem/block.hpp>
#include <core/mem/pool.hpp>
#include <core/mem/manager.hpp>
#include <core/io/manager.hpp>
#include <core/cpu/registers.hpp>
#include <core/cpu/processor.hpp>
#include <core/debug/debugger.hpp>
#include <core/partition.hpp>
#include <core/scheduler.hpp>

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
		auto GetMemPool() -> mem::Pool&;
		auto GetMemManager() -> mem::Manager&;
		auto GetIoManager() -> io::Manager&;
		auto GetVcManager() -> vmc::Manager&;
		auto GetProcessor(std::uint32_t index_v) -> cpu::Processor&;

		/**********************************
		 *  Processor configuration methods
		 **********************************/
		auto Run() -> void;

		/****************
		 *  Utility stuff
		 ****************/
		static auto GetCapability(WHV_CAPABILITY_CODE, void* buffer_v, std::uint32_t length_v) -> std::uint32_t;
		template <typename T>
		static inline auto GetCapability(WHV_CAPABILITY_CODE code_v) -> T {
			T buffer_v{ };
			[[maybe_unused]] auto const length_v = GetCapability(code_v, &buffer_v, sizeof(buffer_v));
			//if(length_v == sizeof(buffer_v));
			return buffer_v;
		}

	protected:
		friend struct Config;


		/************************
		 *  Misc internal methods
		 ************************/
		auto InitializePartition() -> void;
		auto InitializeProcessor(std::uint32_t index_v) -> void;

		/************************
		 *  Exit handling methods
		 ************************/
		auto DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;
    auto NextInstruction(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> void;
		auto DispatchHalt(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;

		/****************
		 * Internal state
		 ****************/
	private:
		Scheduler m_Scheduler;
		Partition m_Partition;
		mem::Pool m_MemPool;
		mem::Manager m_MemManager;
		io::Manager m_IoManager;
		vmc::Manager m_VcManager;
		std::vector<cpu::Processor> m_Processors;
		debug::Debugger m_Debugger;
	};
}