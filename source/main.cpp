#include <main/hvdosmachine.hpp>

int main(int argc, char** argv) try
{
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");

	HvDosMachine machine_v { argc, argv };
	auto result_v = machine_v.Run();
  return result_v;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
