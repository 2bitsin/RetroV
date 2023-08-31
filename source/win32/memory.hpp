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
	enum alloc_flag: std::uint32_t {
		commit								= MEM_COMMIT,
		reserve								= MEM_RESERVE,
		reset									= MEM_RESET,
		reset_undo						= MEM_RESET_UNDO,
		large_pages						= MEM_LARGE_PAGES,
		physical							= MEM_PHYSICAL,
		top_down							= MEM_TOP_DOWN,
		write_watch						= MEM_WRITE_WATCH
	};

	enum free_flag: std::uint32_t {
		decommit							= MEM_DECOMMIT,
		release								= MEM_RELEASE,
		coalesce_placeholders	= MEM_COALESCE_PLACEHOLDERS,
		preserve_placeholders	= MEM_PRESERVE_PLACEHOLDER
	};

	enum page_prot: std::uint32_t {
		no_access							= PAGE_NOACCESS,
		read_only							= PAGE_READONLY,
		read_write						= PAGE_READWRITE,
		write_copy						= PAGE_WRITECOPY,
		execute								= PAGE_EXECUTE,
		execute_read					= PAGE_EXECUTE_READ,
		execute_read_write		= PAGE_EXECUTE_READWRITE,
		execute_write_copy		= PAGE_EXECUTE_WRITECOPY,
		targets_invalid				= PAGE_TARGETS_INVALID,
		targets_no_update			= PAGE_TARGETS_NO_UPDATE,
		guard									= PAGE_GUARD,
		no_cache							= PAGE_NOCACHE,
		write_combine					= PAGE_WRITECOMBINE
	};


	DEFINE_ENUM_FLAG_OPERATORS(alloc_flag)
	DEFINE_ENUM_FLAG_OPERATORS(page_prot)
	DEFINE_ENUM_FLAG_OPERATORS(free_flag)

	auto virtual_free(void* address_v, std::size_t size_v,
		free_flag flags_v = free_flag::release) -> bool;

	template <typename T>
	struct virtual_span_deleter
	{
		inline auto operator () (::std::span<T>& what_v) -> void {
			if (what_v.data() != nullptr) {
				assert(what_v.size() > 0);
				auto result_v = virtual_free(what_v.data(), 
					0u, free_flag::release);
				assert(result_v == true);
				what_v = ::std::span<T>{};
			}
		}
	};

	template <typename T>
	using unique_span = utils::unique_span<T, virtual_span_deleter<T>>;

	auto virtual_alloc(std::size_t size_v,
		page_prot protect_v = page_prot::read_write,
		alloc_flag flags_v = alloc_flag::commit | alloc_flag::reserve,
		void* target_v = nullptr) -> void*;

	template <typename T = std::byte> requires (std::is_trivial_v<T>)
	static inline auto virtual_alloc_s(std::size_t size_v, page_prot prot_v = page_prot::read_write, 
		alloc_flag flags_v = alloc_flag::commit | alloc_flag::reserve,
		void* base_v = nullptr) -> unique_span<T>
	{
		
		auto size_in_bytes_v = size_v * sizeof(T);
		if (auto data_v = virtual_alloc(size_in_bytes_v, prot_v, flags_v, base_v); data_v)
			return { (T*)data_v, size_v };
		error::throw_last_error();
	}


	auto copy_dirty_pages(std::span<std::byte> target_v, std::span<std::byte> source_v, std::span<std::uint64_t> mask_v) -> std::int32_t;

	auto query_and_reset_dirty_pages(void const* base_v, std::size_t size_v, std::span<void const*> dirty_list_v, bool reset_v = true)
		-> std::tuple<std::int32_t, std::uintptr_t, std::span<void const*>>;
}