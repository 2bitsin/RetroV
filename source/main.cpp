#include <machine/isapc.hpp>
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

	machine::ISAPC isapc_v({argv, (size_t)argc});

	auto tick_v = high_resolution_clock::now() + 1s;
	auto last_v = true;
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