#pragma once

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <span>

#include <core/object.hpp>

namespace core
{
	struct hypervisor;

	struct machine_monitor: public object
	{
		virtual auto io_write(hypervisor& host_v, std::uint16_t port_v, std::uint8_t size_v, std::uint32_t data_v) -> bool = 0;
		virtual auto io_fetch(hypervisor& host_v, std::uint16_t port_v, std::uint8_t size_v, std::uint32_t& data_v) -> bool = 0;
	};

}