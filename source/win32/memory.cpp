#include <win32/memory.hpp>
#include <win32/windows.hpp>

#include <utils/literals.hpp>

#include <algorithm>

using namespace win32;

template <std::size_t Boundry = 8u>
static inline auto is_aligned(std::uintptr_t address_v) noexcept -> bool
{
	return (address_v % Boundry) == 0u;
}

template <std::size_t Boundry = 8u>
static inline auto is_aligned(void const* address_v) noexcept -> bool
{
	return is_aligned<Boundry>((std::uintptr_t)address_v);
}

template <std::size_t Boundry = 8u, typename... T>
static inline auto all_aligned(T&&... address_v) noexcept -> bool
{
	return (is_aligned<Boundry>(std::forward<T>(address_v)) && ...);
}


auto win32::virtual_alloc(std::size_t size_v, page_protection_type prot_v,
	allocation_flags_type flags_v, void* target_v) -> void*
{
	return ::VirtualAlloc(target_v, size_v, (DWORD)flags_v, (DWORD)prot_v);  
}

auto win32::copy_dirty_pages(std::span<std::byte> target_v, 
	std::span<std::byte> source_v, 
	std::span<std::uint64_t> mask_v) -> std::int32_t
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

	std::uint64_t curr_bit_v = 0u;
	std::uint64_t last_bit_v = 0u;

	std::size_t begoff_v = 0u;
	std::size_t offset_v = 0u; 

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

auto win32::virtual_free(void* address_v, std::size_t size_v, 
	free_flags_type flags_v) -> bool
{ 
	if (free_flags_type::release) 
	{
		if (flags_v != free_flags_type::decommit)
			flags_v &= ~free_flags_type::decommit;
		if (size_v != 0u)
			size_v = 0u;		
	}

	return ::VirtualFree(address_v, size_v, 
		std::to_underlying(flags_v));		
}
