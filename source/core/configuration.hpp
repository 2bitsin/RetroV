#pragma once

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <span>

#include <core/access.hpp>
#include <core/object.hpp>

namespace core
{
	struct configuration: public object
	{
		static auto create() -> std::unique_ptr<configuration>;
		virtual auto attach_processor(std::uint32_t index_v) -> void = 0;
		virtual auto attach_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void = 0;
		virtual auto attach_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::span<std::byte> source_v) -> void = 0;
		virtual auto attach_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::span<std::byte const> source_v) -> void = 0;
		virtual auto attach_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::filesystem::path const& source_v) -> void = 0;
	};
}