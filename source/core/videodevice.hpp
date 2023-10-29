#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <list>

#include <core/videodevice/common.hpp>
#include <core/videodevice/ramdac.hpp>
#include <core/videodevice/chargen.hpp>
#include <core/videodevice/crtctrl.hpp>

#include <core/configuration.hpp>
#include <core/mapgparange.hpp>
#include <core/romimage.hpp>
#include <core/display.hpp>

#include <utils/limited_span.hpp>
#include <utils/smart_span.hpp>
#include <utils/region.hpp>
#include <utils/span.hpp>

#include <win32/mappedfile.hpp>
#include <win32/memory.hpp>
#include <win32/error.hpp>

namespace core
{
	struct Machine;	
	struct Processor;	
	struct HypercallContext;

	struct VideoDevice
	{
		using duration_type = videodevice::duration_type;
		using buffer_type = videodevice::buffer_type;
		using region_type = utils::region64_type;
;
	;	using RamDAC = videodevice::RamDAC;
		using CharGen = videodevice::CharGen;
		using CrtCtrl = videodevice::CrtCtrl;

		VideoDevice(Machine& machine_v);
		~VideoDevice();

		auto Initialize(Configuration const& config_v) -> void;
		auto Start() -> void;
		auto Stop() -> void;
		auto Restart() -> void;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 16u> data_v) -> std::int32_t;
		auto Hypercall(Processor const& vcpu_v, HypercallContext const& hypercall_v) -> std::int32_t;

	protected:
		
		auto IoPortWrite(Processor const& vcpu_v, std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t;
		auto IoPortFetch(Processor const& vcpu_v, std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>;
		auto ConfigureROM(core::Configuration const&) -> void;
		auto ConfigureMemory(core::Configuration const&) -> void;
		
		auto Refresh(std::stop_token stopee_v) -> void;
		
	protected:

		template <std::uint16_t _Base = 0x3B0u>
		static inline constexpr auto VgaPort(std::uint16_t value_v) 
			-> std::uint16_t
		{
			if (value_v < _Base)
				throw std::out_of_range{ "port value is out of range" };
			return value_v - _Base;
		}

		static inline constexpr const auto Port_MdaCrtIndex				= VgaPort(0x3B4u);
		static inline constexpr const auto Port_MdaCrtData				= VgaPort(0x3B5u);
		static inline constexpr const auto Port_MdaInputStatus 		= VgaPort(0x3BAu);
		static inline constexpr const auto Port_MdaFeatureControl = VgaPort(0x3BAu);

		static inline constexpr const auto Port_AttributeWrite		= VgaPort(0x3C0u);
		static inline constexpr const auto Port_AttributeRead			= VgaPort(0x3C1u);
		static inline constexpr const auto Port_InputStatus 			= VgaPort(0x3C2u);
		static inline constexpr const auto Port_MiscOutputWrite		= VgaPort(0x3C2u);
		static inline constexpr const auto Port_SequencerIndex		= VgaPort(0x3C4u);
		static inline constexpr const auto Port_SequencerData			= VgaPort(0x3C5u);

		static inline constexpr const auto Port_DacStateRead			= VgaPort(0x3C7u);
		static inline constexpr const auto Port_DacIndexRead			= VgaPort(0x3C7u);
		static inline constexpr const auto Port_DacIndexWrite			= VgaPort(0x3C8u);
		static inline constexpr const auto Port_DacDataRead				= VgaPort(0x3C9u);		
		static inline constexpr const auto Port_DacDataWrite			= VgaPort(0x3C9u);

		static inline constexpr const auto Port_FeatureControl 		= VgaPort(0x3CAu);
		static inline constexpr const auto Port_MiscOutputRead		= VgaPort(0x3CCu);

		static inline constexpr const auto Port_GraphicsCtrlIndex	= VgaPort(0x3CEu);
		static inline constexpr const auto Port_GraphicsCtrlData	= VgaPort(0x3CFu);

		static inline constexpr const auto Port_VgaCrtIndex				= VgaPort(0x3D4u);
		static inline constexpr const auto Port_VgaCrtData				= VgaPort(0x3D5u);
		static inline constexpr const auto Port_VgaInputStatus		= VgaPort(0x3DAu);
		static inline constexpr const auto Port_VgaFeatureControl	= VgaPort(0x3DAu);	


	private:	
		

		Machine& m_Machine;		

		std::optional<core::RomImage> m_BiosRom;
		videodevice::buffer_type m_VideoMemory[2u];

	};
}