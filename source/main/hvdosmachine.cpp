#include <main/hvdosmachine.hpp>

HvDosMachine::HvDosMachine(int argc, char** argv)
{
	using namespace size_literals;
	using namespace core;

	std::filesystem::path path_v { argv[0] };
	path_v = path_v.parent_path() / "..";

	Config config_v;

	config_v.AddProcessor(0x0u);
	config_v.SetMemorySize(32_MiB);

	if (cpu::Processor::IsVendorAMD()) {
		config_v.SetBootROM(0xF0000u, path_v / "ROMs/BiosAMD.bin");
	} else if (cpu::Processor::IsVendorIntel()) {
		config_v.SetBootROM(0xF0000u, path_v / "ROMs/BiosIntel.bin");
	} else {
		throw std::runtime_error("Unknown CPU vendor");
	}

	hypervisor_v.emplace(config_v);
	porte9_v.emplace(*hypervisor_v);
	disk0_v.emplace(*hypervisor_v, 0x00u);	
	video_v.emplace(*hypervisor_v);
}

auto HvDosMachine::Run() -> int try {
	hypervisor_v->Run();
	return 0;
} catch (std::exception const& ex) {
	std::cerr << ex.what() << "\n";
	return -1;
}

