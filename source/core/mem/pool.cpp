#include <core/mem/pool.hpp>
#include <core/hypervisor.hpp>
#include <format>

using core::mem::Pool;

Pool::Pool(core::Hypervisor& hypervisor_v)
{}

auto Pool::FreeBlock(std::size_t index_v) -> void
{
	if (index_v >= m_Blocks.size()) {
		throw std::out_of_range{ std::format(
			"{} : Pages index out of range.", __func__) };
	}
	m_Blocks[index_v].Release();
	m_FreeBlocks.push_back(index_v);
}

auto Pool::GetBlock(std::size_t index_v) -> core::mem::Pages& {
	return m_Blocks[index_v];
}

auto Pool::GetBlock(std::size_t index_v) const -> core::mem::Pages const& {
	if (index_v >= m_Blocks.size()) {
		throw std::out_of_range{ std::format(
			"{} : Pages index out of range.", __func__) };
	}
	return m_Blocks[index_v];
}

auto Pool::GetBlockData(std::size_t index_v) -> std::byte*
{
	if (index_v >= m_Blocks.size()) {
		throw std::out_of_range{ std::format(
			"{} : Pages index out of range.", __func__) };
	}
	return GetBlock(index_v).Data();
}

auto Pool::GetBlockData(std::size_t index_v) const -> std::byte const*
{
	return GetBlock(index_v).Data();
}

auto Pool::GetBlockSize(std::size_t index_v) -> std::size_t
{
	return GetBlock(index_v).Size();
}

auto Pool::GetBlockSize(std::size_t index_v) const -> std::size_t
{
	return GetBlock(index_v).Size();
}

auto Pool::GetBlockView(std::size_t index_v) -> std::span<std::byte>
{
	return GetBlock(index_v).View();
}

auto Pool::GetBlockView(std::size_t index_v) const -> std::span<std::byte const>
{
	return GetBlock(index_v).View();
}
