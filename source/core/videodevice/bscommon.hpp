#pragma once

#include <cstdint>
#include <cstddef>
#include <chrono>

#include <win32/memory.hpp>

#include <utils/smart_span.hpp>
#include <utils/span.hpp>


namespace core
{
	struct Machine;
	struct VideoDevice;
}

namespace core::videodevice
{
	using duration_100ns = std::chrono::microseconds;
	using buffer_type = win32::unique_span<std::byte>;

	struct BsCommon
	{
		BsCommon(Machine& machine_v, VideoDevice& device_v);
		~BsCommon() = default;

	protected:
		Machine& m_Machine;
		VideoDevice& m_Device;
	};
}