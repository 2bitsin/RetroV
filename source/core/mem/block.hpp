#pragma once

#include <win32/windows.hpp>

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <span>

namespace core::mem
{
	struct Block
	{
		static inline constexpr const auto kPageSize = 4096;
	
		Block ();
		Block (std::size_t size_v);
		Block (std::span<std::byte const> data_v, std::size_t size_v = 0u, bool repeat_v = false);
		Block (std::filesystem::path const& path_v, std::size_t size_v = 0u, bool repeat_v = false,
			std::uint64_t offset = 0u, std::size_t length = 0u);
		~Block ();
	
		auto operator= (Block const&) -> Block& = delete;
		auto operator= (Block &&) noexcept -> Block&;
		Block(Block const&) = delete;
		Block(Block &&) noexcept;
	
		auto Data () const noexcept -> std::byte const*;
		auto Data () noexcept -> std::byte*;
		auto Size () const noexcept -> std::size_t;
	
		auto View () const noexcept -> std::span<std::byte const>;
		auto View () noexcept -> std::span<std::byte>;
	
		auto Swap (Block& other_v) noexcept -> void;

		auto Rellocate (std::size_t size_v) -> void;
		auto Release () noexcept -> void;
	
	private:
		std::byte* m_Data { nullptr };
		std::size_t m_Size { 0u };
	};
}