#pragma once

#include <cstdint>
#include <cstddef>
#include <utils/span.hpp>

namespace core
{ 
	class Machine;
	class Processor;
	class LocalApic;

	struct LegacyPic
	{
		LegacyPic (Machine& machine_v, std::uint32_t bsp_index_v);
		
		auto Initialize() -> std::int32_t;

		auto IoPortAccess(bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;

	private:
		Machine& m_Machine;
		Processor& m_Processor;
		LocalApic& m_LocalApic;
	};
}