#include <iostream>
#include <filesystem>

//#include <win32/winhvp.hpp>
//#include <win32/winhvp_partition.hpp>

#include <hvisor/virtual_machine.hpp>

int main(int, char**) try
{
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");

	VirtualMachine virtual_machine_v {
		VirtualMachine::config {
			.memory_size = 4u*1024u*1024u,
			.path_to_bios = R"(bios.bin)"
		}
	};

	virtual_machine_v.Restart();
	virtual_machine_v.Run();

  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
