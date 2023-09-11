#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>

#include <core/mapgparange.hpp>
#include <core/configuration.hpp>


namespace core
{
	struct Machine;
	struct Processor;
	struct VideoDevice;
	struct Display;
}

namespace core::videodevice
{
	using core::VideoDevice;
	using core::Machine;
	using core::Processor;
	using core::Display;

	struct Interface
	{
		using duration_type = std::chrono::microseconds;

		Interface(VideoDevice& host_v, Machine& machine_v);

		Interface(Interface const&) = delete;
		Interface(Interface&&) = delete;
		auto operator=(Interface const&) -> Interface& = delete;
		auto operator=(Interface&&) -> Interface& = delete;

		virtual auto Refresh(Display&) -> duration_type = 0;
		virtual auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t = 0;
		virtual auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t = 0;

		virtual ~Interface() = default;

	protected:
		Machine& m_Machine;
		VideoDevice& m_VidHost;
	};
}