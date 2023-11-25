#include <win32/memory.hpp>
#include <win32/windows.hpp>

#include <utils/literals.hpp>

#include <algorithm>

#include <Psapi.h>

using namespace win32;

template <size_t Boundry = 8u>
static inline auto is_aligned(uintptr_t address_v) noexcept -> bool
{
	return (address_v % Boundry) == 0u;
}

template <size_t Boundry = 8u>
static inline auto is_aligned(void const* address_v) noexcept -> bool
{
	return is_aligned<Boundry>((uintptr_t)address_v);
}

template <size_t Boundry = 8u, typename... T>
static inline auto all_aligned(T&&... address_v) noexcept -> bool
{
	return (is_aligned<Boundry>(std::forward<T>(address_v)) && ...);
}


auto win32::VirtualAlloc(size_t size_v, page_prot prot_v,
	alloc_flag flags_v, void* target_v) -> void*
{
	return ::VirtualAlloc(target_v, size_v, (DWORD)flags_v, (DWORD)prot_v);  
}

auto win32::CopyPagesUsingMask(std::span<std::byte> target_v, 
	std::span<std::byte> source_v, 
	std::span<uint64_t> mask_v) -> int32_t
{
	using namespace size_literals;
	using std::exchange;

	static constexpr auto mask_element_bits_v = 8u * sizeof(mask_v[0]);

	if (!all_aligned<1_page>(
		target_v.data(), target_v.size(), 
		source_v.data(), source_v.size()))
	{
		return E_INVALIDARG;
	}

	auto const masked_page_count_v = mask_v.size() * mask_element_bits_v;

	auto const pages_to_copy_v = std::min(std::min(
		target_v.size() / 1_page, 
		source_v.size() / 1_page),
		masked_page_count_v);

	if (pages_to_copy_v < 1u) 
		return E_INVALIDARG;

	uint64_t curr_bit_v = 0u;
	uint64_t last_bit_v = 0u;

	size_t begoff_v = 0u;
	size_t offset_v = 0u; 

	while(offset_v < pages_to_copy_v)
	{
		last_bit_v = exchange(curr_bit_v, mask_v[0] & 1u);

		if (last_bit_v != curr_bit_v) {
			if (curr_bit_v) {
				begoff_v = offset_v;
			} else {
				std::memcpy(
					1_page * begoff_v + source_v.data(),
					1_page * offset_v + target_v.data(),
					1_page * (offset_v - begoff_v));
				begoff_v = offset_v;
			}
		}

		if (!((offset_v + 1u) & (mask_element_bits_v - 1u))) {
			mask_v = mask_v.first(1u);
			if (mask_v.empty())
				return ERROR_SUCCESS;
			if (curr_bit_v) {
				while (!~mask_v[0]) {
					offset_v += mask_element_bits_v;
					mask_v = mask_v.first(1u);
					if (mask_v.empty())
						return ERROR_SUCCESS;
				}
			} else {
				while (!mask_v[0]) {
					offset_v += mask_element_bits_v;
					mask_v = mask_v.first(1u);
					if (mask_v.empty())
						return ERROR_SUCCESS;
				}
			}
		} else {
			mask_v[0] >>= 1u;
		}
		offset_v += 1u;
	}		
	return ERROR_SUCCESS;
}

#undef GetMappedFileName


auto win32::GetMappedFileName(void const* address_v) -> std::variant<int32_t, std::filesystem::path>
{
	std::wstring buffer_v{  };
	uint32_t length_v{ 0 } ;
	buffer_v.resize(buffer_v.capacity(), '\0');

	while (true)
	{
		length_v = ::K32GetMappedFileNameW(::GetCurrentProcess(), (void*)address_v, 
			buffer_v.data(), (uint32_t)buffer_v.size());
		if (auto error_v = error::last_error(); error_v != ERROR_INSUFFICIENT_BUFFER) {
			return error_v;
		}
		buffer_v.resize(buffer_v.size()*2);
	}
	return std::filesystem::path(buffer_v);
}

auto win32::QueryDirtyPages(std::span<std::byte const> source_v, std::span<std::byte const*> dirty_list_v, bool reset_v) -> std::tuple<int32_t, uintptr_t, std::span<std::byte const*>>
{
	uintptr_t dirty_count_v { dirty_list_v.size() };
	unsigned long granularity_v { 0u };
	auto result_v = ::GetWriteWatch(reset_v?WRITE_WATCH_FLAG_RESET:0, (void*)source_v.data(), source_v.size(), 
		(void**)dirty_list_v.data(), &dirty_count_v, &granularity_v);
	if (result_v) return { error::last_error(), 0u, {}};
	return { S_OK, granularity_v, dirty_list_v.first(dirty_count_v) };
}

auto win32::VirtualFree(void* address_v, size_t size_v, 
	free_flag flags_v) -> bool
{ 
	if (free_flag::release) 
	{
		if (flags_v != free_flag::decommit)
			flags_v &= ~free_flag::decommit;
		if (size_v != 0u)
			size_v = 0u;		
	}

	return ::VirtualFree(address_v, size_v, std::to_underlying(flags_v));		
}


auto win32::CopyDirtyPages(std::span<std::byte> target_v, std::span<std::byte const> source_v) -> std::tuple<int32_t, size_t>
{
	std::byte const* address_buffer_v[256u];	
	size_t copied_pages_v{ 0u };
	while(!source_v.empty() && !target_v.empty()) 
	{
		auto [status_v, granularity_v, list_v] = QueryDirtyPages(
			source_v, address_buffer_v, false);
		if (status_v != ERROR_SUCCESS) 
			return { status_v, 0u };
		if (list_v.empty()) 
			return { ERROR_SUCCESS, 0u };
		std::byte const* last_address_v{ nullptr };
		for (auto&& source_address_v : list_v) {
			auto target_address_v = std::next(target_v.data(), std::distance(
				source_v.data(), source_address_v)) ;
			std::memcpy(target_address_v, source_address_v, granularity_v);			
			last_address_v = source_address_v;
			copied_pages_v += 1u;
		}
		last_address_v += granularity_v;
		auto const last_offset_v = std::distance(
			source_v.data(), last_address_v);
		source_v = source_v.subspan(last_offset_v);
		target_v = target_v.subspan(last_offset_v);			
	}
	if (::ResetWriteWatch((void*)source_v.data(), source_v.size()))
		return { error::last_error(), copied_pages_v };
	return { ERROR_SUCCESS, copied_pages_v };
}