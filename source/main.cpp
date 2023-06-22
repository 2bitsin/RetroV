#include <iostream>
#include <filesystem>

#include <core/memory.hpp>

int main(int, char**) try
{
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");

	char Q [] = "Hello World!";
	std::span<std::byte const> q (reinterpret_cast<std::byte const*>(Q), sizeof(Q) - 1);

	Memory memory0 (0x10000u);
	Memory memory1 (0x10000u, R"(bios.bin)", false);
	Memory memory2 (0x20000u, R"(bios.bin)", true);
	Memory memory3 (0x10000u, q, false);
	Memory memory4 (0x10000u, q, true);



  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
