#include <core/display.hpp>

#include <win32/windows.hpp>
#include <win32/error.hpp>

#include <stdexcept>
#include <chrono>

#include <dwmapi.h>

using core::Display;


Display::Display(Machine& machine_v)
	: m_Machine{ machine_v }
{}

Display::~Display() 
{}

auto Display::Initialize(std::uint16_t width_v, std::uint16_t height_v) -> void
{
	if (nullptr != m_Window) {		
		::SDL_SetWindowSize(m_Window.get(), width_v*2, height_v*2);
		if (nullptr == m_Window.get())
			throw std::runtime_error{ __func__ };
	} else {
		m_Window.reset(::SDL_CreateWindow("x86emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width_v*2, height_v*2, 
			SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI));
	}
}

auto Display::GetWindowSize() const -> std::tuple<std::int32_t, std::int32_t> {
	std::int32_t width_v=0, height_v=0;
	if (nullptr == m_Window.get())
		throw std::runtime_error{ __func__ };
	::SDL_GetWindowSize(m_Window.get(), &width_v, &height_v);
	return { width_v, height_v };
}

auto Display::AcquireSurface(std::uint16_t width_v, std::uint16_t height_v) -> surface_tmp 
{
	std::lock_guard lock_v{ x_SurfaceCache };
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
	status_v = ::SDL_UpdateWindowSurface(m_Window.get());
	if (status_v != 0) {
		throw std::runtime_error{ __func__ };
	}
}

auto Display::FlushSurfaceCache() -> void {
	std::lock_guard lock_v { x_SurfaceCache };
	m_SurfaceCache.clear();
}

auto Display::ReleaseSurface(SDL_Surface* ptr) -> void {
	std::lock_guard lock_v{ x_SurfaceCache };
	if (m_SurfaceCache.size() > kSurfaceCacheSize) {
		::SDL_FreeSurface(ptr);
	}
	m_SurfaceCache.emplace_front(ptr);
}

auto Display::WaitSync() -> void
{
	WIN32_ERROR_ASSERT(::DwmFlush());
}
