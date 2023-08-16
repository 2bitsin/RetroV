#include <utils/filebase.hpp>

namespace utils
{
	std::filesystem::path const G_filename_base = std::filesystem::path{ __FILE__ }
		.parent_path()
		.parent_path()
		.parent_path()
		;

	auto relative_to_base(std::filesystem::path const& path_v) -> std::filesystem::path
	{
		return std::filesystem::relative(path_v, G_filename_base);
	}
}