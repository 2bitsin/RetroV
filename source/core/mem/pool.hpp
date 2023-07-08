#pragma once

#include <core/memoryblock.hpp>
#include <core/hypervisor_fwd.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <mutex>
#include <deque>
#include <span>

namespace core::mem
{
	struct Pool
	{
		Pool (core::Hypervisor&);
		~Pool () = default;

		template <typename... T>
		auto AllocateBlock (T&& ... args_v) -> std::size_t {

			if (!m_FreeBlocks.empty()) {
				auto const index_v = m_FreeBlocks.back();
				m_FreeBlocks.pop_front();
				m_Blocks[index_v] = MemoryBlock(std::forward<T>(args_v)...);
				return index_v;
			}

			auto const index_v = m_Blocks.size();
			m_Blocks.emplace_back(std::forward<T>(args_v)...);
			return index_v;
		}

		auto FreeBlock(std::size_t index_v) -> void;

		auto GetBlock(std::size_t index_v) -> MemoryBlock&;
		auto GetBlockData(std::size_t index_v) -> std::byte*;
		auto GetBlockSize(std::size_t index_v) -> std::size_t;
		auto GetBlockView(std::size_t index_v) -> std::span<std::byte>;

		auto GetBlock (std::size_t index_v) const -> MemoryBlock const&;
		auto GetBlockData(std::size_t index_v) const -> std::byte const*;
		auto GetBlockSize(std::size_t index_v) const -> std::size_t;
		auto GetBlockView(std::size_t index_v) const -> std::span<std::byte const>;

	private:
		std::vector<MemoryBlock> m_Blocks;
		std::deque<std::size_t> m_FreeBlocks;
	};
}