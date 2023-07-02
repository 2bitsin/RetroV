#pragma once

#include <device/interface.hpp>

#include <cstdint>
#include <cstddef>

namespace device
{
	struct PortE9: 
		public device::Interface
	{
		PortE9 (core::Hypervisor&, Config const&);
		~PortE9 ();
		
		PortE9 (const PortE9&) = delete;
		PortE9 (PortE9&&) = delete;
		auto operator = (const PortE9&) -> PortE9& = delete;
		auto operator = (PortE9&&)-> PortE9& = delete;

		auto Emulate(std::stop_token const& token_v) -> void override final;
		auto GetCategory() const noexcept -> device::DeviceCatory override final;
		auto SetRunState(DeviceRunState) -> void override final;
		auto GetRunState() -> DeviceRunState override final;

	private:
		core::Hypervisor* m_Hypervisor;
	};
}