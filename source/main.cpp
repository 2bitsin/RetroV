#include <iostream>
#include <filesystem>

#include <core/memory.hpp>


int main(int, char**) try
{
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");

	std::string hello_world_v = "Hello, World!";
	std::span<std::byte const> hello_world_bytes_v{ (std::byte const*)hello_world_v.data(), hello_world_v.size() };



  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
