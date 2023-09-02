#include <utils/paths.hpp>

#include <win32/error.hpp>
#include <win32/windows.hpp>

namespace utils
{
	auto module_filename() -> std::filesystem::path 
	{
		std::wstring buffer_v(MAX_PATH, '\0');
		std::uint32_t result_v{ 0u };
		std::int32_t win32_error_v{ 0 };
	repeat_again:
		result_v = ::GetModuleFileNameW(nullptr, buffer_v.data(), buffer_v.size());

		if (result_v >= buffer_v.size() || result_v < 1u) {
			win32_error_v = win32::error::last_error();
			if (ERROR_INSUFFICIENT_BUFFER == win32_error_v) {
				buffer_v.resize(buffer_v.size() * 2u);
				goto repeat_again;
			}
			if (ERROR_SUCCESS != win32_error_v) {
				throw std::system_error(win32_error_v, std::system_category(), "GetModuleFileNameW");
			}
		}
		buffer_v.resize(result_v);
	  return buffer_v;
	}

	auto build_path(std::filesystem::path const& path_v) -> std::filesystem::path
	{
		std::filesystem::path new_path_v;
		auto const self_path_v = module_filename();
		auto const bin_path_v = self_path_v.parent_path();
		auto const base_path_v = bin_path_v.parent_path();

		for (auto const& part_v : path_v) {
			if (part_v == L"@bin") {
				new_path_v /= bin_path_v; 
			} else if (part_v == L"@base") {
				new_path_v /= base_path_v;
			} else {
				new_path_v /= part_v;
			}
		}
		
		return new_path_v;
	}

	std::filesystem::path const G_source_directory = std::filesystem::path{ __FILE__ }
		.parent_path()
		.parent_path()
		.parent_path()
		;

	auto make_relative_to_source_directory(std::filesystem::path const& path_v) -> std::filesystem::path
	{
		return std::filesystem::relative(path_v, G_source_directory);
	}
}