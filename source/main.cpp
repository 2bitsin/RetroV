#include <main/hvdosmachine.hpp>

#include <SDL2/SDL.h>

#include <filesystem>
#include <cstdlib>
#include <cassert>
#include <chrono>

#undef main
int main(int argc, char** argv) try
{
	using namespace std::chrono_literals;
	using namespace std::chrono;

	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);	
	auto window_v = SDL_CreateWindow("Hypervisor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
	
	std::stop_source stop_source_v;
	

	std::thread t { [&] {
		static constexpr auto const frame_delta_v = 1000000us / 15;
		auto next_frame_v = high_resolution_clock::now() + frame_delta_v;
		std::uint8_t pixel_value_v = 0xFFu;
		std::stop_token stop_token_v{ stop_source_v.get_token() };
		auto buffer_v = SDL_GetWindowSurface(window_v);
		while (!stop_token_v.stop_requested())
		{
			if (high_resolution_clock::now() < next_frame_v)
				std::this_thread::sleep_until(next_frame_v);
			next_frame_v += frame_delta_v;

			auto const pixel_v = SDL_MapRGB(buffer_v->format, pixel_value_v, pixel_value_v, pixel_value_v);
			SDL_FillRect(buffer_v, nullptr, pixel_v);
			SDL_UpdateWindowSurface(window_v);
			pixel_value_v = ~pixel_value_v;
		}
	}};
	

	while (true)
	{
		SDL_Event event_v;
		if (SDL_PollEvent(&event_v)) {
			if (event_v.type == SDL_QUIT) { break; }
			continue;
		}
	}
	stop_source_v.request_stop();
	t.join();

	SDL_DestroyWindow(window_v);


	//std::filesystem::current_path(R"(F:\Archive\FloppyImages)");
	//HvDosMachine hypervisor_v { argc, argv };
	//hypervisor_v.MountImage("DSKA0003.MS-DOS.622.Disk1.img", true);
	//auto result_v = hypervisor_v.Run();
  //return result_v;
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