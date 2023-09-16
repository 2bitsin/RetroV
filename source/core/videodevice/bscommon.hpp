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
	using duration_type = std::chrono::microseconds;
	using buffer_type = win32::unique_span<std::byte>;

	struct BsCommon
	{
		BsCommon(core::Machine& machine_v, core::VideoDevice& device_v);
		~BsCommon() = default;

	private:
		core::Machine& m_Machine;
		core::VideoDevice& m_Device;
	};
}