#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>
#include <win32/whvemulator.hpp>

#include <utils/bitmanip.hpp>
#include <utils/span.hpp>

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

	struct Processor: 
		public win32::WHvProcessor
	{
		using exit_result_type = std::tuple<std::int32_t, WHV_RUN_VP_EXIT_CONTEXT>;
		using exit_future_type = std::shared_future<exit_result_type>;

		Processor(Machine& machine_v, std::uint32_t vcpuindex_v);
		~Processor();

		auto IoPortAccess(bool is_write_v, std::uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t;
		auto GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t;
		auto TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_v, std::uint64_t& physaddr_v) const -> std::int32_t;

		auto UnhandledMsr(WHV_VP_EXIT_CONTEXT const& context_v, WHV_X64_MSR_ACCESS_CONTEXT const& access_v) -> std::int32_t;
		auto UnhandledException(WHV_VP_EXIT_CONTEXT const& context_v, WHV_VP_EXCEPTION_CONTEXT const& exception_v) -> std::int32_t;

		auto Run(std::stop_token stoppee_v) -> exit_result_type;
		auto RunAsync() -> exit_future_type;
		auto CancelAsync() -> void;		
		auto Unsuspend() -> void;
		auto ReadTsc() const -> 
			std::tuple<std::int32_t, std::uint64_t>;

		using WHvProcessor::MemoryFetch;
		using WHvProcessor::MemoryWrite;

	protected:

 		static auto Emulator () -> win32::WHvEmulator&;

		auto InterruptsEnabled() const -> bool;
		auto AdvanceInstruction(WHV_VP_EXIT_CONTEXT const& vpcontext_v) const -> std::int32_t;
	private:
		Machine& m_Machine;		
		std::mutex m_IsRunning;
		std::binary_semaphore m_Suspend;
		std::stop_source m_Stopper;
		exit_future_type m_FutureExit;
	};
}