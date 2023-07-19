#include <core/hostmemory.hpp>
#include <core/constants.hpp>
#include <win32/error.hpp>

#include <system_error>
#include <filesystem>
#include <iostream>
#include <fstream>

using core::HostMemory;

HostMemory::HostMemory()
	: m_Data(nullptr)
	, m_Size(0)
{}

HostMemory::HostMemory(std::size_t size_v)
	: HostMemory()
{
	if (size_v > 0u) HostMemory::Rellocate(size_v);
}

HostMemory::HostMemory(std::span<std::byte const> data_v, std::size_t size_v, bool repeat_v)
	: HostMemory()
{
	if (0u == size_v) {
		size_v = data_v.size();
	}

	if (size_v > 0u) {
		HostMemory::Rellocate(size_v);
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

HostMemory::HostMemory(std::filesystem::path const& path_v, std::size_t size_v, bool repeat_v, std::uint64_t offset_v, std::size_t length_v)
	: HostMemory()
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
	
	HostMemory::Rellocate(size_v);

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

auto HostMemory::Release() noexcept -> void
{
	if (m_Data) {
		VirtualFree(m_Data, 0, MEM_RELEASE);
	}
	m_Data = nullptr;
	m_Size = 0;
}

HostMemory::~HostMemory() 
{
	HostMemory::Release();
}

auto HostMemory::operator=(HostMemory&& prev_v) noexcept -> HostMemory&
{
	if (this==&prev_v)
		return *this;
	auto temp_v(std::move (prev_v));
	swap(temp_v);	
	return *this;
}

auto HostMemory::swap(HostMemory& prev_v) noexcept -> void
{
	std::swap(m_Data, prev_v.m_Data);
	std::swap(m_Size, prev_v.m_Size);
}

auto HostMemory::Rellocate(std::size_t size_v) -> void
{
	HostMemory::~HostMemory();
	size_v = (size_v + kPageSize - 1) & ~(kPageSize - 1);
	auto data_v = VirtualAlloc(nullptr, size_v,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (nullptr == data_v) {
		throw win32::error();
	}
	m_Data = (std::byte*)data_v;
	m_Size = size_v;
}


HostMemory::HostMemory(HostMemory&& prev_v) noexcept
	: m_Data(std::exchange(prev_v.m_Data, nullptr))
	, m_Size(std::exchange(prev_v.m_Size, 0))
{}

auto HostMemory::Size() const noexcept -> std::size_t
{
	return m_Size;
}

template <typename T>
static inline auto HostMemory_Data(T* m_Data, auto m_Size, auto offset_v, auto length_v) 
	-> std::span<T> 
{
	if (0u == length_v) length_v = m_Size;
	if (offset_v >= m_Size) return {};
	if (offset_v + length_v > m_Size)
		length_v = m_Size - offset_v;
	return { m_Data + offset_v, length_v };
}

auto HostMemory::Data(std::size_t offset_v, std::size_t length_v) const noexcept -> std::span<std::byte const>
{
	return HostMemory_Data(m_Data, m_Size, offset_v, length_v);
}

auto HostMemory::Data(std::size_t offset_v, std::size_t length_v) noexcept -> std::span<std::byte>
{
	return HostMemory_Data(m_Data, m_Size, offset_v, length_v);
}

auto HostMemory::Access(std::uint64_t address_v, bool write_v, 
	std::uint8_t size_v, utils::bytes<8u> &data_v) -> std::uint32_t
{
	while (address_v >= m_Size) {
		address_v -= m_Size; }
	if (write_v) {
		if (address_v + size_v > m_Size) {
			auto const copy_v = m_Size - address_v;
			std::memcpy(m_Data + address_v, data_v, copy_v);
			std::memcpy(m_Data, data_v + copy_v, size_v - copy_v);
		} else {
			std::memcpy(m_Data + address_v, data_v, size_v); }
	} else {
		if (address_v + size_v > m_Size) {
			auto const copy_v = m_Size - address_v;
			std::memcpy(data_v, m_Data + address_v, copy_v);
			std::memcpy(data_v + copy_v, m_Data, size_v - copy_v);
		} else {
			std::memcpy(data_v, m_Data + address_v, size_v); }}
	return S_OK;
}
