#include <win32/win32_error.hpp>
#include <win32/windows.hpp>

#include <core/aligned_memory.hpp>
#include <core/virtual_alloc.hpp>
#include <core/mapped_file.hpp>


auto core::aligned_memory::alignment() -> std::uint64_t
{
	return (std::uint64_t)4096u;
}

auto core::aligned_memory::allocate(std::uint64_t size_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>
{
	return std::make_unique<virtual_alloc_buffer>(size_v, access_v);
}

auto core::aligned_memory::from_bytes(std::span<std::byte const> bytes_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>
{
	auto buffer = allocate(bytes_v.size(), access_v);
	auto locked = buffer->lock_mutable();
	std::copy(bytes_v.begin(), bytes_v.end(), locked.begin());
	buffer->unlock();
	return buffer;
}

auto core::aligned_memory::from_file(std::filesystem::path const& path_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>
{
	return std::make_unique<mapped_file>(path_v, 0, 0, access_v);
}

auto core::aligned_memory::from_file(std::filesystem::path const& path_v, std::uint64_t offset_v, std::uint64_t length_v, std::uint32_t access_v) -> std::unique_ptr<aligned_memory>
{
	return std::make_unique<mapped_file>(path_v, offset_v, length_v, access_v);
}
