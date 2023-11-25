#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvemulator.hpp>
#include <win32/chrono.hpp>

#include <core/accessflags.hpp>
#include <core/eventlog.hpp>

#include <utils/span.hpp>
#include <utils/coqueue.hpp>

#include <shared_mutex>
#include <stop_token>
#include <semaphore>
#include <cstdint>
#include <cstddef>
#include <future>
#include <bitset>
#include <tuple>
#include <span>

namespace core
{
	struct Machine;


	struct HypercallContext
	{	union
		{	uint32_t Function;
			struct
			{	uint32_t Minor : 8;
				uint32_t Major : 8;
				uint32_t _0 : 16;
			};
		};
		union
		{	uint32_t Flags;
			struct
			{	uint32_t RaxUsed : 1;
				uint32_t _1 : 31;
			};
		};
		WHV_HYPERCALL_CONTEXT Hypercall;
		WHV_VP_EXIT_CONTEXT VpContext;
	};

	struct Processor: 
		public win32::WHvProcessor
	{
		using exit_result_type = std::tuple<int32_t, WHV_RUN_VP_EXIT_CONTEXT>;
		using exit_future_type = std::shared_future<exit_result_type>;
		using interjection_type = std::function<void(Processor const&)>;

		Processor(Machine& machine_v, uint32_t vcpuindex_v);
		~Processor();

		auto IoPortAccess(bool is_write_v, uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v) const -> int32_t;
		auto MemoryAccess(bool is_write_v, uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) const -> int32_t;

		auto TranslateAddress(uint64_t vaddress_v, core::Access access_v) const -> std::tuple<int32_t, uint64_t>;

		auto RunToExit(std::stop_token stoppee_v) -> exit_result_type;
		auto Start() -> exit_future_type;
		auto Stop() -> void;		
		auto Resume() -> void;
		auto Suspend() -> void;

		auto Interject(interjection_type what_v) -> void;

		auto GetRuntime() const -> std::tuple<int32_t, uint64_t>;
		auto LastExitTime() const -> win32::filetime_clock::time_point;
		auto CurrentTime() const -> win32::filetime_clock::time_point;

		auto InterruptsEnabled() const -> bool;
		auto PagingEnabled() const -> bool;

		auto SetSingleStepMode(bool is_debug_v) -> void;

		using WHvProcessor::MemoryFetch;
		using WHvProcessor::MemoryWrite;

	protected:		
		auto AdvanceInstruction(WHV_VP_EXIT_CONTEXT const& context_v) const -> int32_t;

		auto HypercallDispatch(WHV_RUN_VP_EXIT_CONTEXT const& context_v) const -> int32_t;
		auto HypercallFunction(WHV_RUN_VP_EXIT_CONTEXT const& context_v, HypercallContext& output_v) const -> int32_t;

 		static auto Emulator () -> win32::WHvEmulator&;
	private:
		Machine& m_Machine;		
		std::mutex m_IsRunning;
		std::binary_semaphore m_Suspend;		
		std::stop_source m_Stopper;
		exit_future_type m_FutureExit;
		win32::filetime_clock::time_point m_LastExitTime;
		utils::coqueue<interjection_type> m_IjQueue;
		static inline const EventLog s_log{ "Processor" };
	};
}