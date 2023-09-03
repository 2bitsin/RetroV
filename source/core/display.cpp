#include <core/display.hpp>
#include <win32/windows.hpp>

#include <stdexcept>
#include <chrono>

using core::Display;


Display::Display(Machine& machine_v)
	: m_Machine{ machine_v }
{}

Display::~Display() 
{}

auto Display::Initialize(std::uint16_t width_v, std::uint16_t height_v) -> void
{
	m_Window.reset(::SDL_CreateWindow("x86emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_v*2, height_v*2, 
		SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI));
	if (nullptr == m_Window.get())
		throw std::runtime_error{ __func__ };	
	FlushSurfaceCache();
}

auto Display::AcquireSurface(std::uint16_t width_v, std::uint16_t height_v) -> surface_tmp 
{
retry:
	if (m_SurfaceCache.empty()) 
	{
		auto surface_p = ::SDL_CreateRGBSurface(0, width_v, height_v, 32u, 0, 0, 0, 0);
		if (nullptr == surface_p) 
			throw std::runtime_error{ __func__ };		
		return { surface_p, *this };
	}

	auto surface_p = std::move(m_SurfaceCache.back());
	m_SurfaceCache.pop_back();
	if (surface_p->w != width_v || surface_p->h != height_v) 
	{
		surface_p.reset();
		goto retry;
	}
	return { surface_p.release(), *this };
}

auto Display::Present(surface_tmp surface_v) -> void
{
	auto winsfc_p = ::SDL_GetWindowSurface(m_Window.get());
	if (nullptr == winsfc_p) {
		throw std::runtime_error{ __func__ };
	}
	auto status_v = ::SDL_UpperBlitScaled(surface_v.get(), nullptr, winsfc_p, nullptr);
	if (status_v != 0) {
		throw std::runtime_error{ __func__ };
	}
}

auto Display::FlushSurfaceCache() -> void {
	m_SurfaceCache.clear();
}

auto Display::ReleaseSurface(SDL_Surface* ptr) -> void {
	if (m_SurfaceCache.size() > kSurfaceCacheSize) {
		::SDL_FreeSurface(ptr);
	}
	m_SurfaceCache.emplace_front(ptr);
}
