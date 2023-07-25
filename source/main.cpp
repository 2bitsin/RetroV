#include <SDL2/SDL.h>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/whvcapabilities.hpp>
#include <win32/whvregisters.hpp>

#include <core/virtualmachine.hpp>
#include <utils/literals.hpp>
#include <utils/logger.hpp>

#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cassert>
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

	using core::VirtualMachine;
	using core::Configuration;

	VirtualMachine vmcore_v{ Configuration() }; 
	
	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);		

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