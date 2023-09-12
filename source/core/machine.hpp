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
		
		auto SetIRQ(std::uint16_t state_v) -> void;
		
		auto GetProcessor(std::uint32_t vcpuindex_v) -> Processor& { (void)vcpuindex_v; return m_Processor; }
		auto GetPartition() -> win32::WHvPartition& { return m_Partition; }
		auto GetDisplay() -> Display& { return m_Display; }

		auto SuspendAllProcessors() -> void;
		auto ResumeAllProcessors() -> void;
		

	protected:
		friend Processor;
		friend Debugger;		

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> std::int32_t;
		auto Hypercall(Processor const& vcpu_v, WHV_VP_EXIT_CONTEXT const& context_v, WHV_HYPERCALL_CONTEXT const& hypercall_v) -> std::int32_t;
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

		auto HypercallGetFunction(Processor const& vcpu_v, WHV_VP_EXIT_CONTEXT const& context_v, WHV_HYPERCALL_CONTEXT const& hypercall_v)->std::tuple<std::int32_t, std::uint16_t>;

	private:		
    win32::WorkQueue m_WorkQueue;
		win32::WHvPartition m_Partition;		
		win32::unique_span<std::byte> m_MainMemory;
		std::list<MapGpaRange> m_MappedRanges;
		std::list<win32::MappedFile> m_MappedRoms;

		core::Processor m_Processor;
		core::Processor::exit_future_type m_ProcessorExit;
		core::LegacyPic m_LegacyPic;
		core::VideoDevice m_VideoDevice;
		core::Debugger m_Debugger;
		core::Display m_Display;

		static inline const EventLog s_log{ "Machine" };
	};

}
