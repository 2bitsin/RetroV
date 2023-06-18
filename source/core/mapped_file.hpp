#pragma once

#include <win32/windows.hpp>
#include <win32/win32_error.hpp>

#include <core/access.hpp>
#include <core/memory.hpp>

#include <functional>
#include <stdexcept>
#include <vector>
#include <mutex>

namespace core 
{
	struct mapped_file final: 
		public core::aligned_memory
	{
		
		mapped_file (std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v, std::uint32_t access_v);
	 ~mapped_file ();

		auto attach_to (core::hypervisor& host_v, std::uint64_t base_v, std::uint64_t size_v) -> void override final;

		auto lock_mutable (std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte> override final;
		auto lock_constant (std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte const> override final;
		auto unlock () -> void override final;
		auto size () const -> std::uint64_t override final;

	private:

		HANDLE m_file;
		HANDLE m_view;
		std::byte* m_base;
		std::byte* m_buffer;
		std::uint64_t m_length;
		std::uint32_t m_access;
		std::mutex m_locked;
		std::vector<std::function<void()>> m_unmaps;
	};

}