#pragma once

#include <win32/windows.hpp>
#include <core/memory.hpp>

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <span>

namespace core
{
	struct HostMemory final: public Memory
	{
		using Memory::kPageSize;
	
		HostMemory();
		HostMemory(std::size_t size_v);
		HostMemory(std::span<std::byte const> data_v, std::size_t size_v = 0u, bool repeat_v = false);
		HostMemory(std::filesystem::path const& path_v, std::size_t size_v = 0u, bool repeat_v = false,
			std::uint64_t offset = 0u, std::size_t length = 0u);
	 ~HostMemory();

	  auto Rellocate(std::size_t size_v) -> void;
	  auto Release() noexcept -> void;

		auto operator= (HostMemory const&) -> HostMemory& = delete;
		auto operator= (HostMemory &&) noexcept -> HostMemory&;
		HostMemory(HostMemory const&) = delete;
		HostMemory(HostMemory &&) noexcept;
		auto Swap(HostMemory& other_v) noexcept -> void;
	
		auto Size() const noexcept -> std::size_t  override final;
		auto Data(std::size_t length_v=0u, std::size_t offset_v=0u) const noexcept-> std::span<std::byte const> override final;
		auto Data(std::size_t length_v=0u, std::size_t offset_v=0u) noexcept -> std::span<std::byte>  override final;
		auto Access(std::uint64_t address_v, bool write_v, std::uint8_t size_v, std::uint8_t (&data_v) [8]) -> void override final;
	
	private:
		std::byte* m_Data { nullptr };
		std::size_t m_Size { 0u };
	};
}