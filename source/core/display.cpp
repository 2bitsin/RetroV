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

auto Display::Initialize() -> void
{
	m_Window.reset(::SDL_CreateWindow("x86emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640*2, 400*2, 
		SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI));
	if (nullptr == m_Window.get())
		throw std::runtime_error{ __func__ };	
}

auto Display::StartRefresh(std::uint16_t width_v, std::uint16_t height_v, std::uint8_t refresh_v, refresh_callback callback_v) -> void
{
	if (m_Thread.joinable()) m_Thread.join();	
	if (nullptr == m_Window.get()) 
		throw std::runtime_error{ __func__ };
	::SDL_SetWindowSize(m_Window.get(), width_v, height_v);	 
	m_Thread = std::jthread([this](std::stop_token token_v, std::uint16_t width_v, std::uint16_t height_v,
		std::uint8_t refresh_v, refresh_callback callback_v) {
		return RenderThread(token_v, width_v, height_v, refresh_v, std::move(callback_v));
	}, width_v, height_v, refresh_v, std::move(callback_v));	
}

auto Display::RenderThread(std::stop_token token_v, std::uint16_t width_v, std::uint16_t height_v, std::uint8_t refresh_v, refresh_callback callback_v) -> void
{
	using namespace std::chrono_literals;
	using std::chrono::steady_clock;
	auto interval_v = 1s / (1.0 * refresh_v);
	auto next_v = steady_clock::now() + interval_v;
	while (!token_v.stop_requested())
	{
		auto surface_v = AcquireSurface(width_v, height_v);
		callback_v(surface_v);
		DisplaySurface(std::move(surface_v));
		std::this_thread::sleep_until(next_v);
		next_v += interval_v;
	}
}

auto Display::AcquireSurface(std::uint16_t width_v, std::uint16_t height_v) -> surface_tmp 
{
retry:
	if (m_SurfacePool.empty()) 
	{
		auto surface_p = ::SDL_CreateRGBSurface(0, width_v, height_v, 32u, 0, 0, 0, 0);
		if (nullptr == surface_p) 
			throw std::runtime_error{ __func__ };		
		return { surface_p, *this };
	}

	auto surface_p = std::move(m_SurfacePool.back());
	m_SurfacePool.pop_back();
	if (surface_p->w != width_v || surface_p->h != height_v) 
	{
		surface_p.reset();
		goto retry;
	}
	return { surface_p.release(), *this };
}

auto Display::ReleaseSurface(surface_tmp surface_v) -> void { 
	return ReleaseSurface(surface_v.release()); 
}

auto Display::DisplaySurface(surface_tmp surface_v) -> void
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

auto Display::ReleaseSurface(SDL_Surface* ptr) -> void {
	m_SurfacePool.emplace_front(ptr);
}
