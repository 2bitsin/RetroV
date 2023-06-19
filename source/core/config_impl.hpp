#pragma once

#include <core/config.hpp>

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <variant>
#include <vector>
#include <span>

namespace core
{
	struct config_impl: public config
	{
		config_impl() = default;
		~config_impl() = default;

		struct processor_item
		{
			std::uint32_t index;
		};
	
		struct memory_from_bytes_item
		{
			std::uint64_t base;
			std::uint64_t size;
			std::uint32_t access;
			std::span<std::byte const> source;
		};
	
		struct memory_from_file_item
		{
			std::uint64_t base;
			std::uint64_t size;
			std::uint32_t access;
			std::filesystem::path source;
		};
	
		using config_item_type = std::variant<processor_item, memory_from_bytes_item, memory_from_file_item>;		
		auto items() const -> std::span<config_item_type const>;

	// config::* overrides
		auto add_processor(std::uint32_t index_v) -> void override final;
		auto add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void override final;
		auto add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::span<std::byte const> source_v) -> void override final;
		auto add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::filesystem::path const& source_v) -> void override final;

	private:
		std::vector<config_item_type> m_config;
	};
}