#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/capstone.hpp>
#include <core/cpu/registers.hpp>
#include <core/hypervisor_fwd.hpp>
#include <core/cpu/processor_fwd.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core::debug
{
	struct Debugger
	{
		using Processor = cpu::Processor;
		using RegisterFile = cpu::RegisterFile;


		auto operator = (Debugger const&) -> Debugger& = delete;
		Debugger (Debugger const&) = delete;

		Debugger (Hypervisor& hypervisor_v);
		auto operator = (Debugger&&) noexcept -> Debugger&;
		Debugger (Debugger&&) noexcept ;

		auto Swap(Debugger& other_v) noexcept -> void;

		~Debugger() = default;

		auto PrintRegisters(std::ostream& output_v, RegisterFile const& R) -> void;
		auto Disassemble(std::ostream& output_v, Processor& processor_v, std::uint64_t virtual_address_v, std::size_t count_v) -> void;
	private:
		Hypervisor* m_Hypervisor;
		capstone::instance m_Capstone16;
		capstone::instance m_Capstone32;
		capstone::instance m_Capstone64;
	};
} 