#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <functional>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <mutex>

#include <core/hypervisor_fwd.hpp>
#include <core/cpu/processor_fwd.hpp>



namespace core::io
{
	struct Manager 
	{
		using write_callback = bool(core::Hypervisor&, cpu::Processor&, std::uint16_t port_v, std::uint32_t  data_v, std::uint8_t size_v);
		using fetch_callback = bool(core::Hypervisor&, cpu::Processor&, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v);

		Manager(core::Hypervisor& hypervisor_v);

		auto RegisterWriteCallback(std::uint16_t port_v, write_callback callback_v) -> void;
		auto RegisterFetchCallback(std::uint16_t port_v, fetch_callback callback_v) -> void;

		auto UnregisterWriteCallback(std::uint16_t port_v) -> void;
		auto UnregisterFetchCallback(std::uint16_t port_v) -> void;

		auto DispatchWrite(cpu::Processor& processor_v, std::uint16_t port_v, std::uint32_t data_v, std::uint8_t size_v) -> bool;
		auto DispatchFetch(cpu::Processor& processor_v, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v) -> bool;

		auto DispatchExit(cpu::Processor& processor_v, WHV_RUN_VP_EXIT_CONTEXT const& exit_v) -> bool;

	private:
		Hypervisor& m_Hypervisor;
		std::vector<std::function<write_callback>> m_IoWriteCallback;
		std::vector<std::function<fetch_callback>> m_IoFetchCallback;
	};

}