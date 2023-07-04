#include <machine/isapc.hpp>
#include <device/interface.hpp>
#include <utils/literals.hpp>

using machine::ISAPC;

auto ISAPC::InitializeConfig(std::span<char const* const> args_v) -> core::Config
{
	using namespace size_literals;
	core::Config config_v;
	auto base_path_v = std::filesystem::path(args_v[0]).parent_path().parent_path();
	config_v.SetBootROM(0xF0000u, base_path_v / "ROMs" / "BiosAMD.bin");
	config_v.AddProcessor(0);
	config_v.SetMemorySize(4_MiB);
	return config_v;
}


ISAPC::ISAPC(std::span<char const * const> args_v)
	: m_Hypervisor(InitializeConfig(args_v))
	, m_Devices()
{
	using namespace device;
	std::string s;
	m_Devices.emplace_back(CreateDevice(m_Hypervisor, "porte9", s));
	m_Devices.emplace_back(CreateDevice(m_Hypervisor, "vgadevice", s));
}

ISAPC::~ISAPC()
{}

auto ISAPC::PowerOn() -> void
{
	for (auto&& device_v : m_Devices) {
		device_v->Resume(); }
}

auto ISAPC::Shutdown() -> void
{
	for (auto&& device_v : m_Devices) {
		device_v->Pause(); }
}
