#pragma once

#include <core/hypervisor.hpp>

#include <SDL2/SDL.h>

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct BiosVideo
	{
		BiosVideo(Hypervisor& hypervisor_v);
		~BiosVideo();

		BiosVideo(BiosVideo&&) = delete;
		BiosVideo(BiosVideo const&) = delete;
		auto operator = (BiosVideo&&) -> BiosVideo& = delete;
		auto operator = (BiosVideo const&) -> BiosVideo& = delete;

		auto Int10h(core::Hypervisor& hypervisor_v, core::RegisterFile& registers_v, cpu::Processor& processor_v) -> bool;

	private:
		Hypervisor& m_Hypervisor;
		std::uint32_t m_Int10h { 0u };
		SDL_Window* m_Window { nullptr };
	};
}
