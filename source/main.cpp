#include <utils/literals.hpp>
#include <utils/interval.hpp>
#include <utils/interval_map.hpp>

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

	using utils::interval_map;
	using utils::interval;

	interval_map<std::uint64_t, std::uint64_t> map_v;
	std::uint64_t uinique_id_v{ 0 };

	map_v.insert({ 0x0200u, 0x0300u }, uinique_id_v += 1u);
	map_v.insert({ 0x0000u, 0x0100u }, uinique_id_v += 1u);
	map_v.insert({ 0x0300u, 0x0400u }, uinique_id_v += 1u);
	map_v.insert({ 0x0100u, 0x0200u }, uinique_id_v += 1u);

	map_v.insert({ 0x0010u, 0x0220u }, uinique_id_v += 1u);



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