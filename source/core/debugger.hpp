#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/span.hpp>

#include <core/eventlog.hpp>

#include <cstdint>
#include <cstddef>
#include <string>
#include <mutex>

namespace core
{
	struct Machine;
	struct Processor;
	struct HypercallContext;

	struct Debugger
	{
		Debugger(Machine& machine_v);

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;
		auto Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;
		auto Reset() -> void;
		
	protected:
		auto Hypercall_UnrealModeEnable(Processor const& vcpu_v, bool enable) -> std::int32_t;
		auto Hypercall_WriteLogString(Processor const& vcpu_v, std::uint64_t addr_v, std::uint64_t length_v) -> std::int32_t;
		auto Hypercall_DebuggerBreak(Processor const& vcpu_v, std::uint64_t lin_v, std::uint16_t seg_v, std::uint64_t off_v) -> std::int32_t;
		auto Hypercall_WriteLogChar(Processor const& vcpu_v, char value_v) -> std::int32_t;

		auto FetchMemory(Processor const& vcpu_v, std::uint64_t address_v, std::uint64_t length_v, 
			std::vector<std::byte>& buffer_v) -> std::int32_t;

		auto WriteLogString(Processor const& vcpu_v, std::string_view message_v) -> void;

	private:		
		Machine& m_Machine;
		std::mutex m_Mutex;
		std::string m_Buffer;
		static inline EventLog const s_log{ "Debugger" };
	};
}