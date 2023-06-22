#include <iostream>
#include <filesystem>

#include <core/machine.hpp>

int main(int, char**) try
{
	using namespace size_literals;
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");

	core::Config config_v;

	config_v.AddProcessor(0x0u);
	config_v.SetMemorySize(32_MiB);
	config_v.SetBootROM(0xF0000u, "BIOS.BIN");

	core::Machine machine_v(config_v);

	

  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
