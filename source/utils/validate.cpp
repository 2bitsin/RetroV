#include <utils/validate.hpp>

#include <system_error>
#include <filesystem>
#include <stdexcept>
#include <format>

auto utils::validate_binary(std::filesystem::path const& path_v, std::size_t size_multiple_of_v, std::size_t min_size_v, std::size_t max_size_v) -> void
{ 
	using namespace std;
	using namespace filesystem;

	using enum errc;

	if (!exists(path_v)) throw system_error{ make_error_code(no_such_file_or_directory), path_v.generic_string() };
	if (!is_regular_file(path_v)) throw runtime_error{ format("ROM not a file: {}", path_v.generic_string()) };
	auto const file_size_v = file_size(path_v);
	min_size_v *= size_multiple_of_v;
	max_size_v *= size_multiple_of_v;
	if (file_size_v < min_size_v || file_size_v > max_size_v || (file_size_v % size_multiple_of_v)) {
		throw runtime_error{ format("Invalid video ROM size: {} bytes, "
					"has to be a multiple of {} and between {} and {} bytes in size", 
					file_size_v, size_multiple_of_v, min_size_v, max_size_v) };			
	}
}
