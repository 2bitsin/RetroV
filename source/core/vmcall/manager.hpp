#pragma once

#include <functional>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>

namespace core
{
	struct Hypervisor;
}

namespace core::vmcall
{
	struct Manager
	{
		using vmcall_callback = bool(core::Hypervisor& hypervisor_v, std::uint32_t index_v, std::uint16_t vmcallno_v);

		Manager(core::Hypervisor&);
		~Manager();
		
		auto RegisterVMCall(std::uint16_t callno_v, std::function<vmcall_callback> callback_v) -> std::uint32_t;
		auto UnregisterVMCall(std::uint16_t callno_v, std::uint32_t slot_v) -> void;
		auto DispatchVMCall(std::uint32_t index_v, std::uint16_t callno_v) -> bool;

	private:
		core::Hypervisor& m_Hypervisor;
		std::vector<std::map<std::uint32_t, std::function<vmcall_callback>> m_Callbacks;
	};
}