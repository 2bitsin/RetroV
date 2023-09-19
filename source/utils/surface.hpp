#pragma once

#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <tuple>
#include <span>

#include <SDL2/SDL.h>

namespace utils
{

	template <typename T>
	struct surface_view
	{
		template <typename T>
		inline surface_view(T const& surface_holder_v) noexcept 
			requires requires { { surface_holder_v.get() } -> std::same_as<SDL_Surface*>; }
			: surface_view(surface_holder_v.get())
		{}

		inline surface_view(SDL_Surface* surface_v) noexcept			
			: m_surface(surface_v)
		{
			if (0 != ::SDL_LockSurface(m_surface))
				surface_v = nullptr;
		}

		inline ~surface_view() noexcept
		{
			if (nullptr != m_surface)
				::SDL_UnlockSurface(m_surface);
		}
		
		surface_view(surface_view const&) = delete;
		auto operator=(surface_view const&) -> surface_view& = delete;
		surface_view(surface_view&&) = delete;
		auto operator=(surface_view&&) -> surface_view& = delete;

		inline auto operator [] (std::size_t index_v) -> std::span<T> {
			if (m_surface == nullptr || index_v >= m_surface->h)
				throw std::out_of_range("surface_view::operator[]");
			auto line_addr_v = ((std::byte*)m_surface->pixels) + index_v * (std::uintptr_t)m_surface->pitch;
			return std::span<T>((T*)line_addr_v, (std::uintptr_t)m_surface->pitch / sizeof(T));
		}

		inline auto operator [] (std::size_t index_v) const -> std::span<T> {
			if (m_surface == nullptr || index_v >= m_surface->h)
				throw std::out_of_range("surface_view::operator[]");
			auto line_addr_v = ((std::byte const*)m_surface->pixels) + index_v * (std::uintptr_t)m_surface->pitch;
			return std::span<T const>((T const*)line_addr_v, (std::uintptr_t)m_surface->pitch / sizeof(T));
		}

		inline auto operator [] (std::tuple<std::uint32_t, std::uint32_t> const& index_v) -> T& {
			auto const [yy, xx] = index_v;
			if (m_surface == nullptr || yy >= m_surface->h || xx >= m_surface->w)
				throw std::out_of_range("surface_view::operator[]");
			return ((*this)[yy])[xx];
		}

		inline auto operator [] (std::tuple<std::uint32_t, std::uint32_t> const& index_v) const -> T& {
			auto const [yy, xx] = index_v;
			if (m_surface == nullptr || yy >= m_surface->h || xx >= m_surface->w)
				throw std::out_of_range("surface_view::operator[]");
			return ((*this)[yy])[xx];
		}

		auto cbegin() const noexcept -> T const* {
			return (T const*)m_surface->pixels;
		}

		auto cend() const noexcept -> T const* {
			return (T const*)m_surface->pixels + m_surface->h * m_surface->pitch / sizeof(T);
		}

		auto begin() const noexcept -> T* {
			return (T*)m_surface->pixels;
		}

		auto end() const noexcept -> T* {
			return (T *)m_surface->pixels + m_surface->h * m_surface->pitch / sizeof(T);
		}

	private:
		SDL_Surface* m_surface { nullptr };
	};

}