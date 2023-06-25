#include <main/hvdosmachine.hpp>

HvDosMachine::HvDosMachine(int argc, char** argv)
{
	using namespace size_literals;
	std::filesystem::path path_v { argv[0] };
	path_v = path_v.parent_path() / "..";

	core::Config config_v;
	config_v.AddProcessor(0x0u);
	config_v.SetMemorySize(32_MiB);

	if (core::Machine::IsVendorAMD()) {
		config_v.SetBootROM(0xF0000u, path_v / "ROMs/BiosAMD.bin");
	}
	else if (core::Machine::IsVendorIntel()) {
		config_v.SetBootROM(0xF0000u, path_v / "ROMs/BiosIntel.bin");
	}
	else {
		throw std::runtime_error("Unknown CPU vendor");
	}

	machine_v.emplace(config_v);
	pe9h_device_v.emplace(*machine_v);
}

auto HvDosMachine::Run() -> int try {
	machine_v->Run();
	return 0;
} catch (std::exception const& ex) {
	std::cerr << ex.what() << "\n";
	return -1;
}

