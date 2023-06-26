#pragma once

#include <cstdint>
#include <cstddef>

#include <core/registerfile.hpp>

namespace core 
{
	struct Machine;
	struct VCHandler {		
		static inline constexpr const auto kLastCall = std::uint16_t{ 0xFFFF };

		virtual ~VCHandler() = default;
		virtual auto VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v, std::uint16_t callno_v) -> bool = 0;
		virtual auto VMCall(Machine& machine_v, std::uint32_t cpuindex_v, RegisterFile& registers_v) -> bool {
			VMCall(machine_v, cpuindex_v, registers_v, kLastCall);
		}
	};
}