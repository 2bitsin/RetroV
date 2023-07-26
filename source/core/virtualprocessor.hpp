#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/bitmanip.hpp>

#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct VirtualMachine;

	struct VirtualProcessor
	{
		VirtualProcessor(VirtualMachine& machine_v, std::uint32_t vcpuindex_v);
		~VirtualProcessor();

		auto IoPortAccess(bool is_write_v, std::uint16_t addr_v, std::uint8_t size_v, utils::bytes<4u>& data_v) -> std::int32_t;
		auto MemoryAccess(bool is_write_v, std::uint64_t addr_v, std::uint8_t size_v, utils::bytes<8u>& data_v) -> std::int32_t;

		auto GetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE> values_v) -> std::int32_t;
		auto SetRegisters(std::span<WHV_REGISTER_NAME const> names_v, std::span<WHV_REGISTER_VALUE const> values_v) -> std::int32_t;

		auto TranslateGvaPage(std::uint64_t virtaddr_v, WHV_TRANSLATE_GVA_FLAGS flags_v, WHV_TRANSLATE_GVA_RESULT_CODE& code_v, std::uint64_t& physaddr_v) -> std::int32_t;
	private:
		VirtualMachine& m_Machine;		
		std::uint32_t m_VcpuIndex;
	};
}