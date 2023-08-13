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

	std::vector<std::int64_t> q;

	q.reserve(0xDEAD);
	q.resize(0xBEEF);
	

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
				case SDLK_DELETE: vmcore_v.Reset(); break;
				case SDLK_PAGEUP: vmcore_v.Start(); break;
				case SDLK_PAGEDOWN: vmcore_v.Stop(); break;
				
				case SDLK_0: vmcore_v.RaiseIRQ(0); break;
				case SDLK_1: vmcore_v.RaiseIRQ(1); break;
				case SDLK_2: vmcore_v.RaiseIRQ(2); break;
				case SDLK_3: vmcore_v.RaiseIRQ(3); break;
				case SDLK_4: vmcore_v.RaiseIRQ(4); break;
				case SDLK_5: vmcore_v.RaiseIRQ(5); break;
				case SDLK_6: vmcore_v.RaiseIRQ(6); break;
				case SDLK_7: vmcore_v.RaiseIRQ(7); break;
				case SDLK_8: vmcore_v.RaiseIRQ(8); break;
				case SDLK_9: vmcore_v.RaiseIRQ(9); break;
				case SDLK_F1: vmcore_v.RaiseIRQ(10); break;
				case SDLK_F2: vmcore_v.RaiseIRQ(11); break;
				case SDLK_F3: vmcore_v.RaiseIRQ(12); break;
				case SDLK_F4: vmcore_v.RaiseIRQ(13); break;
				case SDLK_F5: vmcore_v.RaiseIRQ(14); break;
				case SDLK_F6: vmcore_v.RaiseIRQ(15); break;
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