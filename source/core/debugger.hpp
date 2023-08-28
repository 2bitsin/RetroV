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

	struct Debugger
	{
		Debugger(Machine& machine_v);

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;

		auto Reset() -> void;

	private:		
		Machine& m_Machine;
		std::mutex m_Mutex;
		std::string m_Buffer;

		static inline EventLog const s_log{ "Debugger" };
	};
}