#include <core/capabilities.hpp>
#include <machine/isamachine.hpp>
#include <device/interface.hpp>
#include <utils/literals.hpp>


using machine::ISAMachine;

auto ISAMachine::InitializeConfig(std::span<char const* const> args_v) -> core::Config
{
	static const bool is_amd_v = core::Capabilities::IsVendorAMD();
	static const bool is_intel_v = core::Capabilities::IsVendorIntel();

	using namespace size_literals;
	core::Config config_v;
	auto const base_path_v = std::filesystem::path(args_v[0]).parent_path().parent_path();
	auto const bios_path_v = base_path_v / "ROMs" / (is_intel_v ? "BiosIntel.bin" : "BiosAMD.bin");
	config_v.SetBootROM(0xF0000u, bios_path_v);
	config_v.AddProcessor(0);
	config_v.AddProcessor(1);
	config_v.SetMemorySize(4_MiB);
	return config_v;
}


ISAMachine::ISAMachine(std::span<char const * const> args_v)
	: m_Hypervisor(InitializeConfig(args_v))
	, m_Devices()
{
	using namespace device;
	std::string s;
	m_Devices.emplace_back(CreateDevice(m_Hypervisor, "porte9", s));
	m_Devices.emplace_back(CreateDevice(m_Hypervisor, "vgadevice", s));
}

ISAMachine::~ISAMachine()
{}

auto ISAMachine::PowerOn() -> void
{
	m_Hypervisor.PowerOn();
}

auto ISAMachine::Shutdown() -> void
{
	m_Hypervisor.Shutdown();
}
