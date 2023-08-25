#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <span>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/smart_span.hpp>

namespace win32
{
	enum allocation_flags_type: std::uint32_t {
		commit			= MEM_COMMIT,
		reserve			= MEM_RESERVE,
		reset				= MEM_RESET,
		reset_undo	= MEM_RESET_UNDO,

		large_pages	= MEM_LARGE_PAGES,
		physical		= MEM_PHYSICAL,
		top_down		= MEM_TOP_DOWN,
		write_watch	= MEM_WRITE_WATCH
	};

	enum page_protection_type: std::uint32_t {
		no_access						= PAGE_NOACCESS,
		read_only						= PAGE_READONLY,
		read_write					= PAGE_READWRITE,
		write_copy					= PAGE_WRITECOPY,
		execute							= PAGE_EXECUTE,
		execute_read				= PAGE_EXECUTE_READ,
		execute_read_write	= PAGE_EXECUTE_READWRITE,
		execute_write_copy	= PAGE_EXECUTE_WRITECOPY,
		targets_invalid			= PAGE_TARGETS_INVALID,
		targets_no_update		= PAGE_TARGETS_NO_UPDATE,
		guard								= PAGE_GUARD,
		no_cache						= PAGE_NOCACHE,
		write_combine				= PAGE_WRITECOMBINE
	};


	DEFINE_ENUM_FLAG_OPERATORS(allocation_flags_type)
	DEFINE_ENUM_FLAG_OPERATORS(page_protection_type)

	auto virtual_free(void* address_v, std::size_t size_v,
		allocation_flags_type flags_v = allocation_flags_type::reset) -> bool;

	template <typename T>
	struct virtual_span_deleter
	{
		inline auto operator () (::std::span<T>& what_v) -> void {
			if (what_v.data() != nullptr) {
				assert(what_v.size() > 0);
				auto result_v = virtual_free(what_v.data(), what_v.size()*sizeof(T), 
					allocation_flags_type::reset);
				assert(result_v == true);
				what_v = ::std::span<T>{};
			}
		}
	};

	auto virtual_alloc(std::size_t size_v,
		allocation_flags_type flags_v = allocation_flags_type::commit | allocation_flags_type::reserve,
		page_protection_type protect_v = page_protection_type::read_write,
		void* target_v = nullptr) -> void*;

	template <typename T = std::byte> requires (std::is_trivial_v<T>)
	static inline auto virtual_alloc_s(std::size_t size_v, allocation_flags_type flags_v = allocation_flags_type::commit | allocation_flags_type::reserve,
		page_protection_type protect_v = page_protection_type::read_write,
		void* target_v = nullptr) -> utils::unique_span<T, virtual_span_deleter<T>>
	{
		return { (T*)virtual_alloc(size_v*sizeof(T), flags_v, protect_v, target_v), size_v };
	}

}