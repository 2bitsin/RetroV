#include <SDL2/SDL.h>

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/whvcapabilities.hpp>
#include <win32/whvregisters.hpp>
#include <win32/workqueue.hpp>
#include <win32/memory.hpp>
#include <win32/mappedfile.hpp>

#include <core/machine.hpp>

#include <utils/smart_span.hpp>
#include <utils/literals.hpp>
#include <utils/logger.hpp>
#include <utils/paths.hpp>

#include <string_view>
#include <filesystem>
#include <iostream>
#include <string>
#include <format>
#include <chrono>
#include <cstdio>
#include <format>
#include <chrono>

static inline auto MakeConfiguration() -> core::Configuration 
{
	auto config_v = core::Configuration();

	config_v.SetProperty("video.memory.size.kilobytes", "1024");
	config_v.SetProperty("video.rom.path", "@roms/@vendor/video.bin");	

	config_v.SetProperty("system.memory.size.megabytes", "64");
	config_v.SetProperty("system.rom.path", "@roms/@vendor/system.bin");

	return config_v;
}


#undef main
int main(int argc, char** argv) try
{	
	using namespace size_literals;
	using namespace std::chrono_literals;
	using namespace std::chrono;
	using namespace std::filesystem;

	using namespace win32;

	using std::chrono::steady_clock;

	using core::Machine;
	using core::Configuration;

	current_path(path(argv[0])
		.parent_path()
		.parent_path());
	
#if 1
	WHvCapabilities::InfoDump();

	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);


	Machine vmcore_v{ MakeConfiguration() };
	vmcore_v.Start();


	std::uint16_t IRQstate_v{ 0 };
	std::uint16_t last_IRQstate_v{ 0 };

	SDL_Event event_v {};

	while (true)
	{

		if (SDL_PollEvent(&event_v))
		{
			if (event_v.type == SDL_QUIT) { break; }
			switch (event_v.type)
			{
			case SDL_KEYDOWN:
				switch (event_v.key.keysym.sym) 
				{
				case SDLK_DELETE:   vmcore_v.Reset(); break;
				case SDLK_PAGEUP:   vmcore_v.Start(); break;
				case SDLK_PAGEDOWN: vmcore_v.Stop(); break;

				case SDLK_PAUSE:    vmcore_v.GetProcessor(0).Suspend(); break;
				case SDLK_ESCAPE:   vmcore_v.GetProcessor(0).Resume(); break;

				case SDLK_0:  IRQstate_v |= (1u << 0u ); break;
				case SDLK_1:  IRQstate_v |= (1u << 1u ); break;
				case SDLK_2:  IRQstate_v |= (1u << 3u ); break;
				case SDLK_3:  IRQstate_v |= (1u << 4u ); break;
				case SDLK_4:  IRQstate_v |= (1u << 5u ); break;
				case SDLK_5:  IRQstate_v |= (1u << 6u ); break;
				case SDLK_6:  IRQstate_v |= (1u << 7u ); break;
				case SDLK_7:  IRQstate_v |= (1u << 8u ); break;
				case SDLK_8:  IRQstate_v |= (1u << 9u ); break;
				case SDLK_9:  IRQstate_v |= (1u << 10u); break;
				case SDLK_F1: IRQstate_v |= (1u << 11u); break;
				case SDLK_F2: IRQstate_v |= (1u << 12u); break;
				case SDLK_F3: IRQstate_v |= (1u << 13u); break;
				case SDLK_F4: IRQstate_v |= (1u << 14u); break;
				case SDLK_F5: IRQstate_v |= (1u << 15u); break;							
				default: break;
				}
				continue;
			case SDL_KEYUP:
				switch (event_v.key.keysym.sym)
				{
				case SDLK_0:  IRQstate_v &= ~(1u << 0u ); break;
				case SDLK_1:  IRQstate_v &= ~(1u << 1u ); break;
				case SDLK_2:  IRQstate_v &= ~(1u << 3u ); break;
				case SDLK_3:  IRQstate_v &= ~(1u << 4u ); break;
				case SDLK_4:  IRQstate_v &= ~(1u << 5u ); break;
				case SDLK_5:  IRQstate_v &= ~(1u << 6u ); break;
				case SDLK_6:  IRQstate_v &= ~(1u << 7u ); break;
				case SDLK_7:  IRQstate_v &= ~(1u << 8u ); break;
				case SDLK_8:  IRQstate_v &= ~(1u << 9u ); break;
				case SDLK_9:  IRQstate_v &= ~(1u << 10u); break;
				case SDLK_F1: IRQstate_v &= ~(1u << 11u); break;
				case SDLK_F2: IRQstate_v &= ~(1u << 12u); break;
				case SDLK_F3: IRQstate_v &= ~(1u << 13u); break;
				case SDLK_F4: IRQstate_v &= ~(1u << 14u); break;
				case SDLK_F5: IRQstate_v &= ~(1u << 15u); break;
				default: break;
				}
				continue;
			}
			continue;
		}
		if (last_IRQstate_v != IRQstate_v)
		{
			vmcore_v.SetIRQ(IRQstate_v);
			last_IRQstate_v = IRQstate_v;
		}
		vmcore_v.RunMain();
	}

	vmcore_v.Stop();
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