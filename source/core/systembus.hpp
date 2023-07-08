#pragma once

#include <core/hypervisor_fwd.hpp>
#include <core/processor_fwd.hpp>
#include <core/mem/

#include <cstdint>
#include <cstddef>

namespace core
{
	struct SystemBus
	{
		SystemBus(Hypervisor&);
		~SystemBus();

		auto MmWrite(Processor& vcpu_v, std::uint64_t addr_v, std::uint64_t  data_v, std::uint8_t size_v) -> void;
		auto MmFetch(Processor& vcpu_v, std::uint64_t addr_v, std::uint64_t& data_v, std::uint8_t size_v) -> void;
		auto IoWrite(Processor& vcpu_v, std::uint16_t addr_v, std::uint32_t  data_v, std::uint8_t size_v) -> void;
		auto IoFetch(Processor& vcpu_v, std::uint16_t addr_v, std::uint32_t& data_v, std::uint8_t size_v) -> void;


		auto MmMap(Pages&

	private:
		Hypervisor* m_Hypervisor{ nullptr };
	};
}