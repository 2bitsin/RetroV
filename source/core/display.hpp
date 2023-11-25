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
		static inline constexpr const auto kSurfaceCacheSize = 3u;

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

		Display(Machine& machine_v);
		~Display();
			
		auto Initialize(uint16_t width_v = 640, uint16_t height_v = 400) -> void;
		auto GetWindowSize() const->std::tuple<int32_t, int32_t>;
		auto AcquireSurface(uint16_t width_v, uint16_t height_v) -> surface_tmp;		
		auto Present(surface_tmp surface_v) -> void;
		auto FlushSurfaceCache() -> void;
		auto WaitSync() -> void;

	protected:
		friend struct surface_releaser;
		auto ReleaseSurface(SDL_Surface* ptr) -> void;		

	private:
		Machine& m_Machine;
		window_ptr m_Window;
		std::mutex x_SurfaceCache;
		std::deque<surface_ptr> m_SurfaceCache;
	};
}