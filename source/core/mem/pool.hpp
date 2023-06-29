#pragma once

#include <core/mem/block.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <mutex>

namespace core::mem
{
	struct Pool
	{
		Pool () = default;
		~Pool () = default;

		template <typename... T>
		auto AllocateBlock (T&& ... args_v) -> std::size_t {
			auto const index_v = m_Blocks.size();
			m_Blocks.emplace_back(std::forward<T>(args_v)...);
			return index_v;
		}

		auto GetBlock (std::size_t index_v) -> Block&;
		auto GetBlockData(std::size_t index_v) -> std::byte*;
		auto GetBlockSize(std::size_t index_v) -> std::size_t;
		auto GetBlockView(std::size_t index_v) -> std::span<std::byte>;

		auto GetBlock (std::size_t index_v) const -> Block const&;
		auto GetBlockData(std::size_t index_v) const -> std::byte const*;
		auto GetBlockSize(std::size_t index_v) const -> std::size_t;
		auto GetBlockView(std::size_t index_v) const -> std::span<std::byte const>;

	private:
		std::vector<Block> m_Blocks;
	};
}