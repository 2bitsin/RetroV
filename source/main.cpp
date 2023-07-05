#include <utils/interval_map.hpp>
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

	std::uint64_t unique_id { 0 };
	utils::interval_map<uint64_t, uint64_t> map_v;

	map_v.insert({ 20u, 100u}, unique_id += 1u);
	map_v.insert({100u, 200u}, unique_id += 1u);
	map_v.insert({200u, 300u}, unique_id += 1u);
	map_v.insert({200u, 400u}, unique_id += 1u);
	map_v.insert({300u, 400u}, unique_id += 1u);
	map_v.insert({450u, 550u}, unique_id += 1u);
	map_v.insert({550u, 650u}, unique_id += 1u);
	map_v.insert({650u, 750u}, unique_id += 1u);
	map_v.insert({310u, 850u}, unique_id += 1u);

	std::uint64_t q0, q1, q2, q3, q4 { 0 };
	try {
		q0 = map_v.at(200u);
		q1 = map_v.at(220u);
		q2 = map_v.at(300u);
		q3 = map_v.at(849u);
		q4 = map_v.at( 10u);
	} catch (std::exception const& ex) {
		std::cout << ex.what() << "\n";
		__debugbreak();
	}

	__debugbreak();
	//SDL_Init(SDL_INIT_EVERYTHING);
	//std::atexit(SDL_Quit);		

	//machine::ISAPC isapc_v({argv, (size_t)argc});
	//while (true)
	//{
	//	SDL_Event event_v;
	//	if (SDL_PollEvent(&event_v)) {
	//		if (event_v.type == SDL_QUIT) { break; }
	//		continue;
	//	}
	//}
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