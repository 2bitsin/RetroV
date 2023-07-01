#include <machine/genericisapc.hpp>
#include <device/genericvga.hpp>
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

	auto base_path_v = std::filesystem::path(argv[0]).parent_path();
	config_v.SetBootROM(0xF0000u, (base_path_v / "ROMs") / "BiosAMD.bin");
	config_v.AddProcessor(0);
	config_v.SetMemorySize(4_MiB);

	core::Hypervisor hypervisor_v(config_v);
	device::GenericVGA vga_v(hypervisor_v, config_v);

	std::stop_source stop_source_v;
	auto stop_token_v = stop_source_v.get_token();
	std::thread _ { [&] {
		vga_v.Emulate(stop_token_v);
	}};

	while (true)
	{
		SDL_Event event_v;
		if (SDL_PollEvent(&event_v)) {
			if (event_v.type == SDL_QUIT) { break; }
			continue;
		}
	}

	_.join();

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