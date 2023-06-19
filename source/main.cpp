#include <iostream>
#include <filesystem>

#include <core/hypervisor.hpp>
#include <core/machine_monitor.hpp>

struct simple_machine_monitor final:
	public core::machine_monitor
{
	auto io_write(core::hypervisor& host_v, std::uint16_t port_v, std::uint8_t size_v, std::uint32_t data_v) -> bool override final
	{
		switch (port_v)
		{
		case 0xE9:
			std::cout << (char)data_v << "\n"; 
			return true;
		default: 
			return false;
		}
	}

	auto io_fetch(core::hypervisor& host_v, std::uint16_t port_v, std::uint8_t size_v, std::uint32_t& data_v) -> bool override final
	{		
		return false;
	}

	auto initialize(core::config& config_v) -> void override final
	{
		config_v.add_processor(0);

		config_v.add_memory(0x00000000u, 0x000A0000u, core::access::all);                // 000000 ... 09FFFF RAM
		config_v.add_memory(0x000A0000u, 0x00020000u, core::access::device);             // 0A0000 ... 0BFFFF VGA/Text
	//config_v.add_memory(0x000C0000u, 0x00010000u, core::access::rom);                // 0C0000 ... 0CFFFF Adapter ROM
	//config_v.add_memory(0x000D0000u, 0x00010000u, core::access::rom);                // 0D0000 ... 0DFFFF Adapter ROM
	//config_v.add_memory(0x000E0000u, 0x00010000u, core::access::rom);                // 0E0000 ... 0EFFFF Adapter ROM
		config_v.add_memory(0x000F0000u, 0x00010000u, core::access::rom, R"(bios.bin)"); // 0F0000 ... 0FFFFF BIOS
		config_v.add_memory(0x00100000u, 0x00360000u, core::access::rom);                // 100000 ... 460000 (4MB - 640KB) of RAM
	}
};


int main(int, char**) try
{
	std::filesystem::current_path(R"(C:\Users\alex\Desktop\projects\leisure\HvDOS\workspace)");


	auto virtual_machine_v = core::hypervisor::create(
		std::make_unique<simple_machine_monitor>());
	virtual_machine_v->start_machine();
  return 0;
}
catch (std::exception const& ex) 
{
	std::cerr << ex.what() << "\n";
	return -1;
}
