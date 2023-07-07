#include <core/mem/block.hpp>
#include <win32/error.hpp>

#include <system_error>
#include <filesystem>
#include <iostream>
#include <fstream>

using core::mem::Block;

Block::Block()
	: m_Data(nullptr)
	, m_Size(0)
{
}

Block::Block(std::size_t size_v)
	: Block()
{
	if (size_v > 0u) Block::Rellocate(size_v);
}

Block::Block(std::span<std::byte const> data_v, std::size_t size_v, bool repeat_v)
	: Block()
{
	if (0u == size_v) {
		size_v = data_v.size();
	}

	if (size_v > 0u) {
		Block::Rellocate(size_v);
	}

	if (repeat_v) {
		std::size_t offset_v = 0;
		while (offset_v < size_v) {
			std::size_t copy_v = std::min(size_v - offset_v, data_v.size());
			std::memcpy(m_Data + offset_v, data_v.data(), copy_v);
			offset_v += copy_v;
		}
	} else {
		std::memcpy(m_Data, data_v.data(), std::min(size_v, data_v.size()));
	}
}

Block::Block(std::filesystem::path const& path_v, std::size_t size_v, bool repeat_v, std::uint64_t offset_v, std::size_t length_v)
	: Block()
{
	if (!std::filesystem::exists(path_v)) 
	{
		throw std::system_error(std::make_error_code(
			std::errc::no_such_file_or_directory));
	}

	auto file_size_v = std::filesystem::file_size(path_v);

	if (0u == length_v) length_v = file_size_v - offset_v;	

	if (offset_v + length_v > file_size_v || !length_v) {
		throw std::system_error(std::make_error_code(
			std::errc::invalid_argument));
	}

	if (0u == size_v) size_v = length_v;
	
	Block::Rellocate(size_v);

	std::ifstream file_v(path_v, std::ios::binary);
	if (!file_v) throw std::system_error(std::make_error_code(std::errc::io_error));	
	file_v.seekg(offset_v);
	if (!file_v) throw std::system_error(std::make_error_code(std::errc::io_error));
	file_v.read((char*)m_Data, std::min(length_v, size_v));		

	if (repeat_v && length_v < size_v) 
	{
		std::size_t offset_v = length_v;
		while (offset_v < size_v) 
		{
			std::size_t copy_v = std::min(size_v - offset_v, length_v);
			std::memcpy(m_Data + offset_v, m_Data, copy_v);
			offset_v += copy_v;
		}
	}
}

auto Block::Release() noexcept -> void
{
	if (m_Data) {
		VirtualFree(m_Data, 0, MEM_RELEASE);
	}
	m_Data = nullptr;
	m_Size = 0;
}

Block::~Block() 
{
	Block::Release();
}

auto Block::operator=(Block&& prev_v) noexcept -> Block&
{
	if (this==&prev_v)
		return *this;
	auto temp_v(std::move (prev_v));
	Swap(temp_v);	
	return *this;
}

auto Block::Swap(Block& prev_v) noexcept -> void
{
	std::swap(m_Data, prev_v.m_Data);
	std::swap(m_Size, prev_v.m_Size);
}

auto Block::Rellocate(std::size_t size_v) -> void
{
	Block::~Block();
	size_v = (size_v + kPageSize - 1) & ~(kPageSize - 1);
	auto data_v = VirtualAlloc(nullptr, size_v,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (nullptr == data_v) {
		throw win32::error();
	}
	m_Data = (std::byte*)data_v;
	m_Size = size_v;
}


Block::Block(Block&& prev_v) noexcept
	: m_Data(std::exchange(prev_v.m_Data, nullptr))
	, m_Size(std::exchange(prev_v.m_Size, 0))
{}

auto Block::Data() const noexcept -> std::byte const*
{
	return m_Data;
}

auto Block::Data() noexcept -> std::byte *
{
	return m_Data;
}

auto Block::Size() const noexcept -> std::size_t
{
	return m_Size;
}

auto Block::View() const noexcept -> std::span<std::byte const>
{
	return { m_Data, m_Size };
}

auto Block::View() noexcept -> std::span<std::byte>
{
	return { m_Data, m_Size };
}
