#include <main/hvdosmachine.hpp>

int main(int argc, char** argv) try
{
	std::filesystem::current_path(R"(F:\Archive\FloppyImages)");
	HvDosMachine machine_v { argc, argv };
	machine_v.MountImage("DSKA0003.MS-DOS.622.Disk1.img");
	auto result_v = machine_v.Run();
  return result_v;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
