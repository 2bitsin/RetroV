#include <main/hvdosmachine.hpp>

#include <SDL2/SDL.h>

#include <filesystem>
#include <cstdlib>

#undef main
int main(int argc, char** argv) try
{
	SDL_Init(SDL_INIT_EVERYTHING);
	std::atexit(SDL_Quit);
	std::filesystem::current_path(R"(F:\Archive\FloppyImages)");
	HvDosMachine hypervisor_v { argc, argv };
	hypervisor_v.MountImage("DSKA0003.MS-DOS.622.Disk1.img", true);
	auto result_v = hypervisor_v.Run();
  return result_v;
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