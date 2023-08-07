#include <SDL2/SDL.h>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/whvcapabilities.hpp>
#include <win32/whvregisters.hpp>
#include <win32/workqueue.hpp>

#include <core/machine.hpp>

#include <utils/literals.hpp>
#include <utils/logger.hpp>

#include <string_view>
#include <filesystem>
#include <iostream>
#include <string>
#include <format>
#include <chrono>
#include <cstdio>
#include <format>
#include <chrono>

#undef main
int main(int argc, char** argv) try
{	
	using namespace size_literals;
	using namespace std::chrono_literals;
	using namespace std::chrono;
	using namespace std::filesystem;
	
	current_path(path(argv[0])
		.parent_path()
		.parent_path());

	using namespace win32;

#if 1
	win32::WHvCapabilities::LogInformation();

	using core::Machine;
	using core::Configuration;

	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);

	SDL_Window* window_v = SDL_CreateWindow(
		"Virtual Machine", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		800, 600, 
		SDL_WINDOW_SHOWN);

	Machine vmcore_v{ Configuration() };
	vmcore_v.Start();

	while (true)
	{
		SDL_Event event_v;
		if (SDL_PollEvent(&event_v)) 
		{
			if (event_v.type == SDL_QUIT) { break; }
			if (event_v.type == SDL_KEYDOWN) 
			{
				switch (event_v.key.keysym.sym) 
				{
				case SDLK_F8: vmcore_v.Stop(); break;
				case SDLK_F9: vmcore_v.Start(); break;
				case SDLK_F10: vmcore_v.Reset(); break;
				case SDLK_F11: vmcore_v.Interrupt(0x11); break;
				}
			}
			continue;
		}
		vmcore_v.RunMain();
		SDL_UpdateWindowSurface(window_v);
	}

	vmcore_v.Stop();	
	SDL_DestroyWindow(window_v);
#endif
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