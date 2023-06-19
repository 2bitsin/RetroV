#pragma once

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <span>

#include <core/access.hpp>
#include <core/object.hpp>
#include <core/hypervisor.hpp>

namespace core
{
	struct aligned_memory: public object
	{		
		virtual auto attach_to(hypervisor& host_v, std::uint64_t base_v, std::uint64_t size_v=0) -> void = 0;

		virtual auto lock_mutable(std::uint64_t base_v=0, std::uint64_t size_v=0) -> std::span<std::byte> = 0;
		virtual auto lock_constant(std::uint64_t base_v=0, std::uint64_t size_v=0) -> std::span<std::byte const> = 0;

		virtual auto unlock() -> void = 0;
		virtual auto size() const -> std::uint64_t = 0;

		static auto alignment() -> std::uint64_t;
		static auto allocate(std::uint64_t size_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>;	
		static auto from_bytes(std::span<std::byte const> bytes_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>;
		static auto from_file(std::filesystem::path const& path_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>;
		static auto from_file(std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v, std::uint32_t access_v)->std::unique_ptr<aligned_memory>;
	};

}