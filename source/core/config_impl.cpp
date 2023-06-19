#include <core/config_impl.hpp>

auto core::config_impl::add_processor(std::uint32_t index_v) -> void
{
	m_config.emplace_back(processor_item{ index_v });
}

auto core::config_impl::add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v) -> void
{
	m_config.emplace_back(memory_from_bytes_item{ base_v, size_v, access_v,{} });
}

auto core::config_impl::add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::span<std::byte const> source_v) -> void
{
	m_config.emplace_back(memory_from_bytes_item{ base_v, size_v, access_v, source_v });
}

auto core::config_impl::add_memory(std::uint64_t base_v, std::uint64_t size_v, std::uint32_t access_v, std::filesystem::path const& source_v) -> void
{
	m_config.emplace_back(memory_from_file_item{ base_v, size_v, access_v, source_v });
}

auto core::config_impl::items() const->std::span<config_item_type const>
{
	return m_config;
}
