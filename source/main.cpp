#include <utils/literals.hpp>
#include <utils/interval_map.hpp>
#include <tests/interval_map.hpp>

#include <SDL2/SDL.h>

#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <cstdio>

#undef main
int main(int argc, char** argv) try
{	
	using namespace size_literals;
	using namespace std::chrono_literals;
	using namespace std::chrono;
	
	std::filesystem::current_path(R"(F:\Archive\FloppyImages)");

	TEST_interval_map();

#if 0
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