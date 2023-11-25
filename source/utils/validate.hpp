#pragma once

#include <cstdint>
#include <cstddef>

#include <filesystem>

namespace utils
{

	// Throws if not valid
	auto validate_binary(std::filesystem::path path_v, size_t size_multiple_of_v, size_t min_size_v, size_t max_size_v) 
		-> std::filesystem::path;

}