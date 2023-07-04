#pragma once

#include <device/interface.hpp>
#include <core/cpu/processor_fwd.hpp>
#include <core/scheduler_fwd.hpp>
#include <core/service_fwd.hpp>

#include <SDL2/SDL.h>

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <atomic>
#include <mutex>

namespace device
{
	struct VGADevice final:  
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

		VGADevice(core::Hypervisor& hypervisor_v, Config const& config_v);

		VGADevice(VGADevice const&) = delete;
		VGADevice(VGADevice&&) = delete;
		auto operator=(VGADevice const&) -> VGADevice& = delete;
		auto operator=(VGADevice&&) -> VGADevice& = delete;

		~VGADevice () override;

		auto Emulate(core::Scheduler&, core::Service&) -> void override final;
		auto GetCategory() const noexcept -> device::DeviceCatory override final;
		auto Pause() -> void override final;
		auto Resume() -> void override final;


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