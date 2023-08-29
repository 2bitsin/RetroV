#pragma once

#include <functional>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <thread>
#include <future>
#include <deque>
#include <span>

#include <SDL2/SDL.h>

namespace core
{
	struct Machine;

	struct Display
	{
		struct surface_deleter {
			auto operator()(SDL_Surface* ptr) const noexcept -> void {
				assert(ptr != nullptr);
				::SDL_FreeSurface(ptr);
			}
		};

		struct surface_releaser {
			auto operator()(SDL_Surface* ptr) const noexcept -> void {
				assert(ptr != nullptr);
				m_display.ReleaseSurface(ptr);
			}

			surface_releaser(Display& display_v) noexcept: m_display(display_v) {}

		private:
			Display& m_display;
		};

		struct window_deleter {
			auto operator()(SDL_Window* ptr) const noexcept -> void {
				assert(ptr != nullptr);
				::SDL_DestroyWindow(ptr);
			}
		};

		using surface_ptr = std::unique_ptr<SDL_Surface, surface_deleter>;
		using surface_tmp = std::unique_ptr<SDL_Surface, surface_releaser>;
		using window_ptr = std::unique_ptr<SDL_Window, window_deleter>;
		using refresh_callback = std::function<void(surface_tmp&)>;

		Display(Machine& machine_v);
		~Display();
			
		auto Initialize() -> std::int32_t;
		auto StartRefresh(std::uint16_t width_v, std::uint16_t height_v, 
			std::uint8_t refresh_v, refresh_callback callback_v) -> std::int32_t;
		auto AcquireSurface(std::uint16_t width_v, std::uint16_t height_v) -> surface_tmp;
		auto ReleaseSurface(surface_tmp surface_v) -> void;
		auto DisplaySurface(surface_tmp surface_v) -> void;

	protected:
		friend struct surface_releaser;
		auto ReleaseSurface(SDL_Surface* ptr) -> void;		
		auto RenderThread(std::stop_token stop_v, std::uint16_t width_v, std::uint16_t height_v,
			std::uint8_t refresh_v, refresh_callback callback_v) -> void;

	private:
		Machine& m_Machine;
		std::deque<surface_ptr> m_SurfacePool;
		window_ptr m_Window;
		std::jthread m_Thread;
	};
}