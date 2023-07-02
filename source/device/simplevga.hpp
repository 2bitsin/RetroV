#pragma once

#include <device/interface.hpp>
#include <core/cpu/processor_fwd.hpp>

#include <SDL2/SDL.h>

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <atomic>
#include <mutex>

namespace device
{

	struct SimpleVGA final:  
		public device::Interface
	{	
		enum  RenderingMode :
			std::uint32_t
		{
			kTextMonochrome,
			kTextColor,
			kGraphicsLinear1BppMonochrome,
			kGraphicsLinear2BppIndexed,
			kGraphicsPlanar4BppIndexed,
			kGraphicsPlanar8BppIndexed,
			kGraphicsLinear8BppIndexed,
			kGraphicsLinear16BppRGB,
			kGraphicsLinear24BppRGB,
			kGraphicsLinear32BppRGB			
		};

		SimpleVGA(core::Hypervisor& hypervisor_v, Config const& config_v);

		SimpleVGA(SimpleVGA const&) = delete;
		SimpleVGA(SimpleVGA&&) = delete;
		auto operator=(SimpleVGA const&) -> SimpleVGA& = delete;
		auto operator=(SimpleVGA&&) -> SimpleVGA& = delete;

		~SimpleVGA () override;

		auto Emulate(std::stop_token const& token_v) -> void override final;
		auto GetCategory() const noexcept -> device::DeviceCatory override final;
		auto SetRunState(DeviceRunState) -> void override final;
		auto GetRunState() -> DeviceRunState override final;

		auto SetVideoMode(RenderingMode mode_v, std::uint16_t width_v, std::uint16_t height_v) -> void;

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
		std::size_t m_TaskIndex { 0 };
		RenderingMode m_RenderingMode { RenderingMode::kTextColor };
		std::uint16_t m_Height { 0 };
		std::uint16_t m_Width { 0 };
	};

}