#pragma once

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <span>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/memory.hpp>
#include <utils/region.hpp>

namespace win32
{
	enum share_type: std::uint32_t {
		share_read = FILE_SHARE_READ,
		share_write = FILE_SHARE_WRITE,
		share_delete = FILE_SHARE_DELETE,
		share_all = share_read | share_write | share_delete
	};

	DEFINE_ENUM_FLAG_OPERATORS(share_type)

	enum cf_mode : std::uint32_t {
		create_new = CREATE_NEW,
		create_always = CREATE_ALWAYS,
		open_existing = OPEN_EXISTING,
		open_always = OPEN_ALWAYS,
		truncate_existing = TRUNCATE_EXISTING
	};

	struct MappedFile 
	{
		struct close_handle_type { 
			auto operator()(void const* handle_v) const noexcept { 
				if (INVALID_HANDLE_VALUE!=handle_v&&handle_v) 
					::CloseHandle((void*)handle_v);
			} 
		};

		using unique_handle = std::unique_ptr<void, close_handle_type>;

		MappedFile(std::filesystem::path const& path_v, 
			utils::region_64_t regn_v = {0, 0},
			cf_mode mode_v = cf_mode::open_existing,
			page_prot prot_v = page_prot::execute_read,
			share_type share_v = share_type::share_read);

		~MappedFile() = default;

		MappedFile(MappedFile const&) = delete;
		auto operator=(MappedFile const&) -> MappedFile& = delete;

		MappedFile(MappedFile&&) = default;
		auto operator=(MappedFile&&) -> MappedFile& = default;

		auto Data() const noexcept -> std::span<std::byte>;
		auto Size() const noexcept -> std::size_t;

	private:		
		unique_handle m_File;
		unique_handle m_Mapp;
		std::byte* m_MapPtr;
		std::span<std::byte> m_Data;
	};
}