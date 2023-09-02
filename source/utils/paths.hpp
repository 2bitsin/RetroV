#pragma once

#include <filesystem>


namespace utils {
	extern std::filesystem::path const G_source_directory;
	auto make_relative_to_source_directory(std::filesystem::path const& path_v) -> std::filesystem::path;

	auto build_path(std::filesystem::path const& path_v) -> std::filesystem::path;
	auto module_filename() -> std::filesystem::path;

}