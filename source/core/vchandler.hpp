#pragma once

#include <cstdint>
#include <cstddef>

#include <core/registerfile.hpp>

namespace core 
{
	struct Hypervisor;
	struct VCHandler {		
		static inline constexpr const auto kLastCall = std::uint16_t{ 0xFFFF };

		virtual ~VCHandler() = default;
		virtual auto VMCall(Hypervisor& hypervisor_v, std::uint32_t cpuindex_v, RegisterFile& registers_v, std::uint16_t callno_v) -> bool = 0;
		virtual auto VMCall(Hypervisor& hypervisor_v, std::uint32_t cpuindex_v, RegisterFile& registers_v) -> bool {
			return VMCall(hypervisor_v, cpuindex_v, registers_v, kLastCall);
		}
	};
}