#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

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
		auto InterruptWindow() -> std::int32_t;
		auto AssertLines(std::uint16_t mask_v) -> std::int32_t;
		auto DeassertLines(std::uint16_t mask_v) -> std::int32_t;

	private:
		std::mutex m_lock;
	
		std::uint16_t m_irr { 0u };
		std::uint16_t m_isr { 0u };

		Machine& m_Machine;
		Processor& m_Processor;
	};
}