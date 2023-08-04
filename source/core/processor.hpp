#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/whvprocessor.hpp>

#include <utils/bitmanip.hpp>
#include <utils/span.hpp>

#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct Machine;

	struct Processor: 
		public win32::WHvProcessor
	{
		Processor(Machine& machine_v, std::uint32_t vcpuindex_v);
		~Processor();

		auto IoPortAccess(bool is_write_v, std::uint16_t addr_v, utils::limited_span<std::byte, 4u> data_v) const -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) const -> std::int32_t;
		auto GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) const -> std::int32_t;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) const -> std::int32_t;
		auto TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_v, std::uint64_t& physaddr_v) const -> std::int32_t;

	private:
		Machine& m_Machine;		
	};
}