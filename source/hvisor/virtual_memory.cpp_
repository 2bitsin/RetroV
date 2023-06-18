#include <system_error>
#include <filesystem>
#include <fstream>

#include <hvisor/virtual_memory.hpp>
#include <win32/Hresult.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

VirtualMemory::VirtualMemory()
	: m_data{ nullptr }
	, m_size{ 0u }
{}

VirtualMemory::VirtualMemory(std::size_t size_v)
	: VirtualMemory()
{
	size_v = ((size_v+0x3FFu)&~0x3FFull);
	auto const data_v = ::VirtualAlloc(nullptr, size_v, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (data_v == nullptr) throw win32_error(GetLastError());
	m_data = (std::byte*)data_v;
	m_size = size_v;
}

VirtualMemory::VirtualMemory(std::filesystem::path const& path_v, std::size_t size_limit_v)
	: VirtualMemory(std::filesystem::exists(path_v)
		? std::min(size_limit_v, std::filesystem::file_size(path_v))
		: throw std::system_error(std::make_error_code(
			std::errc::no_such_file_or_directory)))
{
	auto const file_size_v = std::filesystem::file_size(path_v);
	std::ifstream file_v(path_v, std::ios::binary);
	if (!file_v) throw std::system_error(std::make_error_code(
		std::errc::no_such_file_or_directory));
	file_v.read((char*)m_data, std::min(file_size_v, m_size));
	if (file_v.gcount() != file_size_v) throw std::system_error(std::make_error_code(
		std::errc::file_too_large));
}

VirtualMemory::~VirtualMemory()
{
	if (m_data==nullptr||m_size==0) return;
	::VirtualFree(m_data, 0, MEM_RESERVE);
	m_data=nullptr;
	m_size=0u;
}

VirtualMemory::VirtualMemory(VirtualMemory&& prev_v) noexcept
	:	m_data { std::exchange(prev_v.m_data, nullptr) }
	,	m_size { std::exchange(prev_v.m_size, 0u) }
{}

auto VirtualMemory::operator=(VirtualMemory&& prev_v) noexcept -> VirtualMemory&
{
	if (&prev_v==nullptr) return *this;
	auto temp_v { std::move(prev_v) };
	temp_v.swap(*this);
	return *this;
}

auto VirtualMemory::swap(VirtualMemory& prev_v) -> void
{
	std::swap(m_data, prev_v.m_data);
	std::swap(m_size, prev_v.m_size);
}
