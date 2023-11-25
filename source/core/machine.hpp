#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>
#include <win32/whvemulator.hpp>
#include <win32/whvpartition.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/workqueue.hpp>
#include <win32/workqueue.hpp>

#include <core/configuration.hpp>
#include <core/mapgparange.hpp>
#include <core/romimage.hpp>
#include <core/processor.hpp>
#include <core/legacypic.hpp>
#include <core/videodevice.hpp>
#include <core/memory.hpp>
#include <core/debugger.hpp>
#include <core/display.hpp>

#include <utils/region.hpp>
#include <utils/span.hpp>

#include <shared_mutex>
#include <memory>
#include <mutex>
#include <tuple>
#include <list>

struct SDL_Window;

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
		
		auto SetIRQ(uint16_t state_v) -> void;
		
		auto GetProcessor(uint32_t vcpuindex_v) -> Processor& { (void)vcpuindex_v; return m_Processor; }
		auto GetPartition() -> win32::WHvPartition& { return m_Partition; }
		auto GetDisplay() -> Display& { return m_Display; }
		auto GetMemory() -> Memory& { return m_Memory; }

		auto SuspendAllProcessors() -> void;
		auto ResumeAllProcessors() -> void;		

	protected:
		friend Processor;
		friend Debugger;		

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> int32_t;
		auto Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> int32_t;
	
		auto ConfigurePartition(Configuration const&) -> void;
		auto ConfigureMemory(Configuration const&) -> void;

	private:		
		win32::WHvPartition m_Partition;		

		core::Processor m_Processor;
		core::Processor::exit_future_type m_ProcessorExit;
		core::Memory m_Memory;
		core::LegacyPic m_LegacyPic;
		core::VideoDevice m_VideoDevice;
		core::Debugger m_Debugger;
		core::Display m_Display;

		static inline const EventLog s_log{ "Machine" };
	};

}
