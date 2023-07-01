#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <core/cpu/registers.hpp>

#include <functional>
#include <cstdint>
#include <cstddef>
#include <vector>

namespace core
{
	struct Hypervisor;
}

namespace core::cpu
{
	struct Processor;
}

namespace core::vmc
{
	struct Manager
	{
		static inline constexpr const auto kLastCallNumber = 0xFFFFu;

		using vmcall_callback = bool(core::Hypervisor& hypervisor_v, core::RegisterFile& registers_v, cpu::Processor& processor_v, std::uint16_t vmcallno_v);

		Manager(core::Hypervisor&);
		~Manager();

		Manager(Manager const&) = delete;
		auto operator=(Manager const&) -> Manager& = delete;

		Manager(Manager&&) noexcept;
		auto operator=(Manager&&) noexcept -> Manager&;
		auto Swap(Manager&) noexcept -> void;
		
		auto RegisterCallback(std::uint16_t callno_v, std::function<vmcall_callback> callback_v) -> std::uint32_t;
		auto UnregisterCallback(std::uint16_t callno_v, std::uint32_t slot_v) -> void;
		auto DispatchCallback(cpu::Processor& processor_v, std::uint16_t callno_v, WHV_RUN_VP_EXIT_CONTEXT const&) -> bool;
		auto DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const&) -> bool;

	private:
		core::Hypervisor* m_Hypervisor;
		std::uint32_t m_LastID { 1 };
		std::vector<std::vector<std::tuple<std::int64_t, std::function<vmcall_callback>>>> m_Callbacks;
	};
}