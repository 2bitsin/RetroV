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
#include <core/legacypic.hpp>
#include <core/legacyvideo.hpp>
#include <core/debugger.hpp>

#include <utils/span.hpp>
#include <utils/buffer2d.hpp>

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

		auto Render() -> std::tuple<
			LegacyVideo::buffer_type, 
			std::chrono::microseconds>;
		
		auto SetIRQ(std::uint16_t state_v) -> void;
		
		auto GetProcessor(std::uint32_t vcpuindex_v) -> Processor& { (void)vcpuindex_v; return m_Processor; }
		auto GetPartition() -> win32::WHvPartition& { return m_Partition; }

		auto SuspendAllProcessors() -> void;
		auto ResumeAllProcessors() -> void;

	protected:
		friend Processor;
		friend Debugger;


		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;
	
		auto ConfigureMemory(Configuration const&) -> void;
		auto ConfigureBiosROM(Configuration const&) -> void;
		auto ConfigurePartition(Configuration const&) -> void;

	private:		
		win32::WHvPartition m_Partition;
		std::list<Memory> m_Memory;
		core::Processor m_Processor;
		core::Processor::exit_future_type m_ProcessorExit;
		core::LegacyPic m_LegacyPic;
		core::LegacyVideo m_LegacyVideo;
		core::Debugger m_Debugger;
	};

}
