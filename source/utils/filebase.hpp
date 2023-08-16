#pragma once

#include <filesystem>

namespace utils {
	extern std::filesystem::path const G_filename_base;

	auto relative_to_base(std::filesystem::path const& path_v) -> std::filesystem::path;
}