#pragma once

#include <device/interface.hpp>
#include <core/cpu/processor_fwd.hpp>

#include <SDL2/SDL.h>

#include <cstdint>
#include <cstddef>
#include <thread>
#include <atomic>
#include <mutex>

namespace device
{

	struct GenericVGA final:  
		public device::Interface
	{	

		GenericVGA(core::Hypervisor& hypervisor_v, SDL_Window* window_v = nullptr);

		GenericVGA(GenericVGA const&) = delete;
		GenericVGA(GenericVGA&&) = delete;
		auto operator=(GenericVGA const&) -> GenericVGA& = delete;
		auto operator=(GenericVGA&&) -> GenericVGA& = delete;

		~GenericVGA () override = default;

		auto Emulate() -> void override final;

		auto SetVideoMode(std::uint8_t mode_v) -> void;

	protected:
		using Processor = core::cpu::Processor;
		using Hypervisor = core::Hypervisor;

		auto IoWrite(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t  data_v, std::uint8_t size_v) -> bool;
		auto IoFetch(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v) -> bool;

	private:
		Hypervisor* m_Hypervisor { nullptr };
		SDL_Window* m_Window { nullptr };
		std::size_t m_BiosBlock { 0 };
		std::size_t m_VramBlock { 0 };
	};

}