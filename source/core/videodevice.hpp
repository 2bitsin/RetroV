#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <future>
#include <list>

#include <core/configuration.hpp>
#include <core/mapgparange.hpp>
#include <core/romimage.hpp>
#include <core/display.hpp>

#include <utils/limited_span.hpp>
#include <utils/smart_span.hpp>
#include <utils/region.hpp>
#include <utils/span.hpp>

#include <win32/chrono.hpp>
#include <win32/mappedfile.hpp>
#include <win32/memory.hpp>
#include <win32/error.hpp>

namespace core
{
	struct Machine;
	struct Processor;
	struct HypercallContext;

	namespace detail
	{
		template <std::uint16_t _Base = 0x3B0u>
		static inline constexpr auto VgaPort(std::uint16_t value_v)
			-> std::uint16_t
		{
			if (value_v < _Base)
				throw std::out_of_range{ "port value is out of range" };
			return value_v - _Base;
		}
		}

	static inline constexpr const auto Port_MdaCrtIndex = detail::VgaPort(0x3B4u);
	static inline constexpr const auto Port_MdaCrtData = detail::VgaPort(0x3B5u);
	static inline constexpr const auto Port_MdaInputStatus = detail::VgaPort(0x3BAu);
	static inline constexpr const auto Port_MdaFeatureControl = detail::VgaPort(0x3BAu);
	static inline constexpr const auto Port_Attribute0 = detail::VgaPort(0x3C0u);
	static inline constexpr const auto Port_Attribute1 = detail::VgaPort(0x3C1u);
	static inline constexpr const auto Port_InputStatus = detail::VgaPort(0x3C2u);
	static inline constexpr const auto Port_MiscOutputWrite = detail::VgaPort(0x3C2u);
	static inline constexpr const auto Port_SequencerIndex = detail::VgaPort(0x3C4u);
	static inline constexpr const auto Port_SequencerData = detail::VgaPort(0x3C5u);
	static inline constexpr const auto Port_DacPixelMask = detail::VgaPort(0x3C6u);
	static inline constexpr const auto Port_DacStateRead = detail::VgaPort(0x3C7u);
	static inline constexpr const auto Port_DacIndexRead = detail::VgaPort(0x3C7u);
	static inline constexpr const auto Port_DacIndexWrite = detail::VgaPort(0x3C8u);
	static inline constexpr const auto Port_DacDataRead = detail::VgaPort(0x3C9u);
	static inline constexpr const auto Port_DacDataWrite = detail::VgaPort(0x3C9u);
	static inline constexpr const auto Port_FeatureControlRead = detail::VgaPort(0x3CAu);
	static inline constexpr const auto Port_MiscOutputRead = detail::VgaPort(0x3CCu);
	static inline constexpr const auto Port_GraphicsCtrlIndex = detail::VgaPort(0x3CEu);
	static inline constexpr const auto Port_GraphicsCtrlData = detail::VgaPort(0x3CFu);
	static inline constexpr const auto Port_VgaCrtIndex = detail::VgaPort(0x3D4u);
	static inline constexpr const auto Port_VgaCrtData = detail::VgaPort(0x3D5u);
	static inline constexpr const auto Port_VgaInputStatus = detail::VgaPort(0x3DAu);
	static inline constexpr const auto Port_VgaFeatureControl = detail::VgaPort(0x3DAu);

#pragma pack(push, 1)
	struct VideoDeviceStateVga
	{
		VideoDeviceStateVga();

		enum class EValueIndex: std::uint16_t {
			HorizontalTotal,
			HorizontalDisplayEnd,
			HorizontalRetraceStart,
			HorizontalRetraceEnd,
			HorizontalBlankingStart,
			HorizontalBlankingEnd,

			VerticalTotal,
			VerticalDisplayEnd,
			VerticalRetraceStart,
			VerticalRetraceEnd,
			VerticalBlankingStart,
			VerticalBlankingEnd

			


		};

		auto IoPortWrite(std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t;
		auto IoPortFetch(std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>;

		auto GetValue(EValueIndex index_v) const -> std::uint64_t;
		auto SetValue(EValueIndex index_v, std::uint64_t value_v) -> void;


		auto HorizontalTotal() const->uint16_t;
		auto HorizontalDisplayEnd() const->uint16_t;
		auto HorizontalRetraceStart() const->uint16_t;
		auto HorizontalRetraceEnd() const->uint16_t;
		auto HorizontalBlankingStart() const->uint16_t;
		auto HorizontalBlankingEnd() const->uint16_t;

		auto VerticalTotal() const->uint16_t;
		auto VerticalDisplayEnd() const->uint16_t;
		auto VerticalRetraceStart() const->uint16_t;
		auto VerticalRetraceEnd() const->uint16_t;
		auto VerticalBlankingStart() const->uint16_t;
		auto VerticalBlankingEnd() const->uint16_t;

		
		auto DisplayEnableSkew() const -> uint8_t;
		auto HorizontalRetraceSkew() const -> uint8_t;
		auto CursorSkew() const -> uint8_t;

		auto CharacterWidth() const->uint8_t;
		auto CharacterHeight() const->uint8_t;

		auto ScreenDisable() const -> bool;

		auto ScanlineDouble() const -> bool ;
		auto ScanlineClockDivide() const -> bool;

		auto MasterClockRate() const -> uint64_t;
		auto MasterClockDivide() const -> bool;
		auto MemoryClockDivide() const -> bool;
    auto DotClockDivide() const -> bool;

		auto ShiftLoadRate() const -> bool;
		auto ShiftFour() const -> bool;
		auto ByteAddressMode() const -> bool;
		auto OddEventDisable() const -> bool;

		auto ChainOddEven() const -> bool;
		auto ChainFour() const -> bool;
		auto GraphicsMode() const -> bool;
		auto MemoryMapSelect() const -> utils::region32_type;
		auto CharsetA() const -> utils::region32_type;
		auto CharsetB() const -> utils::region32_type;

		auto Log() const -> void;

		struct
		{
			uint8_t index;
			union
			{
				uint8_t data[0x19u];
				struct
				{
					uint8_t horizontal_total;
					uint8_t horizontal_display_end;
					uint8_t horizontal_blanking_start;
					uint8_t horizontal_blanking_end;
					uint8_t horizontal_retrace_start;
					uint8_t horizontal_retrace_end;
					uint8_t vertical_total;
					uint8_t overflow;
					uint8_t preset_row_scan;
					uint8_t maximum_scan_line;
					uint8_t cursor_start;
					uint8_t cursor_end;
					uint8_t start_address_high;
					uint8_t start_address_low;
					uint8_t cursor_location_high;
					uint8_t cursor_location_low;
					uint8_t vertical_retrace_start;
					uint8_t vertical_retrace_end;
					uint8_t vertical_display_end;
					uint8_t offset;
					uint8_t underline_location;
					uint8_t vertical_blanking_start;
					uint8_t vertical_blanking_end;
					uint8_t crt_mode_control;
					uint8_t line_compare;
				};
			};
		} crtctrl;

		struct
		{
			uint8_t index;
			union
			{
				uint8_t data[0x5u];
				struct
				{
					uint8_t reset;
					uint8_t clocking_mode;
					uint8_t map_mask;
					uint8_t character_map_select;
					uint8_t memory_mode;
				};
			};
		} sequencer;

		struct
		{
			uint8_t index;
			union
			{
				uint8_t data[0x9u];
				struct
				{
					uint8_t set_or_reset;
					uint8_t enable_set_or_reset;
					uint8_t color_compare;
					uint8_t data_rotate;
					uint8_t read_map_select;
					uint8_t graphics_mode;
					uint8_t miscellaneous;
					uint8_t color_dont_care;
					uint8_t bit_mask;
				};
			};

		} graphics;

		struct
		{
			uint16_t index;
			uint8_t mask;
			uint8_t latch;
			uint8_t flags;
			uint8_t color[256u * 3u];
		} ramdac;

		struct
		{
			bool latch;

			union
			{
				struct {
					uint8_t index : 5;
					uint8_t pas : 1;
					uint8_t reserved : 2;
				};
				uint8_t index_and_pas;
			};
			union
			{
				uint8_t data[0x15u];
				struct {
					uint8_t palette[0x10u];
					uint8_t mode_control;
					uint8_t overscan_color;
					uint8_t color_plane_enable;
					uint8_t horizontal_panning;
					uint8_t color_select;
				};
			};
		} attrib;
		uint8_t misc_output;
		uint8_t feature_control;

	};
#pragma pack(pop)


	struct VideoDevice
	{
		using duration_type = win32::filetime_clock::duration;
		using buffer_type = win32::unique_span<std::byte>;
		using region_type = utils::region64_type;

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

		auto ConfigureROM(core::Configuration const&) -> void;
		auto ConfigureMemory(core::Configuration const&) -> void;

		auto Refresh(std::stop_token stopee_v) -> void;

	protected:


	private:
		Machine& m_Machine;

		std::optional<RomImage> m_BiosRom;
		buffer_type m_VideoMemory[2u];
		VideoDeviceStateVga m_State[2u];
	};
}