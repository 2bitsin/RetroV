#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core
{
	struct IOHandler 
	{
		virtual ~IOHandler() = default;
		virtual auto PortWrite(std::uint16_t port_v, std::uint64_t  value_v, std::size_t size_v) -> bool = 0;
		virtual auto PortFetch(std::uint16_t port_v, std::uint64_t& value_v, std::size_t size_v) -> bool = 0;
	};
}