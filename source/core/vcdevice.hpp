#pragma once

#include <cstdint>
#include <cstddef>

namespace core 
{
	struct Machine;
	struct VCDevice {
		virtual ~VCDevice() = default;
		virtual void Call(Machine& machine_v, std::uint16_t index_v) = 0;
	};
}