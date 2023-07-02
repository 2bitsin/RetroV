#include <machine/genericisapc.hpp>
#include <device/simplevga.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <cassert>
#include <chrono>

#undef main
int main(int argc, char** argv) try
{
	using namespace size_literals;
	using namespace std::chrono_literals;
	using namespace std::chrono;
	std::filesystem::current_path(R"(F:\Archive\FloppyImages)");

	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);
	
	core::Config config_v;

	auto base_path_v = std::filesystem::path(argv[0])
		.parent_path()
		.parent_path();

	config_v.SetBootROM(0xF0000u, base_path_v / "ROMs" / "BiosAMD.bin");
	config_v.AddProcessor(0);
	config_v.SetMemorySize(4_MiB);

	core::Hypervisor hypervisor_v(config_v);
	device::SimpleVGA vga_v(hypervisor_v, config_v);
	vga_v.SetVideoMode(vga_v.kTextColor, 80u, 25u);
	
	while (true)
	{
		SDL_Event event_v;
		if (SDL_PollEvent(&event_v)) {
			if (event_v.type == SDL_QUIT) { break; }
			continue;
		}
	}
  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
catch (...)
{
	std::cerr << "Unknown exception\n";
	return -1;
}