#pragma once

#include <win32/win32_error.hpp>
#include <win32/windows.hpp>

#include <core/aligned_memory.hpp>

#include <functional>
#include <stdexcept>
#include <vector>
#include <mutex>


namespace core
{
	struct virtual_alloc_buffer final :
		public core::aligned_memory
	{
		virtual_alloc_buffer(std::uint64_t size_v, std::uint32_t access_v);

		~virtual_alloc_buffer();

		auto attach_to(core::hypervisor& host_v, std::uint64_t base_v, std::uint64_t size_v) -> void override final;
		auto lock_mutable(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte> override final;
		auto lock_constant(std::uint64_t base_v, std::uint64_t size_v) -> std::span<std::byte const> override final;
		auto unlock() -> void override final;
		auto size() const -> std::uint64_t override final;

	private:
		std::byte* m_buffer;
		std::uint64_t m_length;
		std::uint32_t m_access;
		std::mutex m_locked;
		std::vector<std::function<void()>> m_unmaps;
	};

}