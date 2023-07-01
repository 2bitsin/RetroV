#include <device/genericvga.hpp>
#include <core/hypervisor.hpp>

#include <functional>
#include <stdexcept>
#include <format>

static inline constexpr std::uint16_t const kFetchPorts[] = {
	0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
	0x03C8, 0x03C9, 0x03CA, 0x03CC, 0x03CE, 0x03CF, 0x03D0, 0x03D1,
	0x03D2, 0x03D3, 0x03D4, 0x03D5, 0x03D6, 0x03DA
};

static inline constexpr std::uint16_t const kWritePorts[] = {
	0x03C0, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8,
	0x03C9, 0x03CE, 0x03CF, 0x03D0, 0x03D1, 0x03D2, 0x03D3, 0x03D4,
	0x03D5, 0x03D6
};

using device::GenericVGA;

GenericVGA::GenericVGA(core::Hypervisor& hypervisor_v, SDL_Window* window_v)
	:	m_Hypervisor(&hypervisor_v)
	, m_Window(window_v)
{
	if (nullptr == window_v) {
		window_v = SDL_CreateWindow("GenericVGA", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			640, 400, 
			SDL_WINDOW_SHOWN);
		if (nullptr == window_v) {
			throw std::runtime_error(std::format("{}: {}\n",
				__func__, SDL_GetError()));
		}
	}
	auto& ioman_v = m_Hypervisor->GetIoManager();
	for (auto const& port_v : kFetchPorts) 
		ioman_v.RegisterFetchCallback(port_v, [this](auto&&...args){return IoFetch(args...);});
	for (auto const& port_v : kWritePorts) 
		ioman_v.RegisterWriteCallback(port_v, [this](auto&&...args){return IoWrite(args...);});
	
}

auto GenericVGA::Emulate() -> void 
{
	::SDL_FillRect(::SDL_GetWindowSurface(m_Window), nullptr, 0xFFFFFFFFu);
	::SDL_UpdateWindowSurface(m_Window);
}

auto GenericVGA::SetVideoMode(std::uint8_t mode_v) -> void
{
}

auto GenericVGA::IoWrite(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t data_v, std::uint8_t size_v) -> bool
{
	return false;
}

auto GenericVGA::IoFetch(Hypervisor& hypervisor_v, Processor& cpu_v, std::uint16_t port_v, std::uint32_t& data_v, std::uint8_t size_v) -> bool
{
	return false;
}
