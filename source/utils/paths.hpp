#pragma once

#include <filesystem>

namespace utils {

	auto path_substitute(std::filesystem::path const& path_v) -> std::filesystem::path;
	auto module_filename() -> std::filesystem::path;

}