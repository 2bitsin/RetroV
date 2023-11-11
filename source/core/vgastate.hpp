#pragma once

#include <cstdint>
#include <cstddef>

#include <type_traits>

#include <utils/logger.hpp>

#include <win32/windows.hpp>

namespace core
{

	namespace detail
	{
		template <std::uint16_t _Base = 0x3B0u>
		static inline constexpr auto port_vga_io(std::uint16_t value_v)
			-> std::uint16_t
		{
			if (value_v < _Base)
				throw std::out_of_range{ "port value is out of range" };
			return value_v - _Base;
		}
	}

	static inline constexpr const auto Port_MdaCrtIndex					= detail::port_vga_io(0x3B4u);
	static inline constexpr const auto Port_MdaCrtData					= detail::port_vga_io(0x3B5u);
	static inline constexpr const auto Port_MdaInputStatus			= detail::port_vga_io(0x3BAu);
	static inline constexpr const auto Port_MdaFeatureControl		= detail::port_vga_io(0x3BAu);
	static inline constexpr const auto Port_Attribute0					= detail::port_vga_io(0x3C0u);
	static inline constexpr const auto Port_Attribute1					= detail::port_vga_io(0x3C1u);
	static inline constexpr const auto Port_InputStatus					= detail::port_vga_io(0x3C2u);
	static inline constexpr const auto Port_MiscOutputWrite			= detail::port_vga_io(0x3C2u);
	static inline constexpr const auto Port_SequencerIndex			= detail::port_vga_io(0x3C4u);
	static inline constexpr const auto Port_SequencerData				= detail::port_vga_io(0x3C5u);
	static inline constexpr const auto Port_DacPixelMask				= detail::port_vga_io(0x3C6u);
	static inline constexpr const auto Port_DacStateRead				= detail::port_vga_io(0x3C7u);
	static inline constexpr const auto Port_DacIndexRead				= detail::port_vga_io(0x3C7u);
	static inline constexpr const auto Port_DacIndexWrite				= detail::port_vga_io(0x3C8u);
	static inline constexpr const auto Port_DacDataRead					= detail::port_vga_io(0x3C9u);
	static inline constexpr const auto Port_DacDataWrite				= detail::port_vga_io(0x3C9u);
	static inline constexpr const auto Port_FeatureControlRead	= detail::port_vga_io(0x3CAu);
	static inline constexpr const auto Port_MiscOutputRead			= detail::port_vga_io(0x3CCu);
	static inline constexpr const auto Port_GraphicsCtrlIndex		= detail::port_vga_io(0x3CEu);
	static inline constexpr const auto Port_GraphicsCtrlData		= detail::port_vga_io(0x3CFu);
	static inline constexpr const auto Port_VgaCrtIndex					= detail::port_vga_io(0x3D4u);
	static inline constexpr const auto Port_VgaCrtData					= detail::port_vga_io(0x3D5u);
	static inline constexpr const auto Port_VgaInputStatus			= detail::port_vga_io(0x3DAu);
	static inline constexpr const auto Port_VgaFeatureControl		= detail::port_vga_io(0x3DAu);

#pragma pack(push, 1)
	struct VgaState
	{
		enum class ValueIndex : std::uint16_t {
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

		constexpr inline VgaState() noexcept {

		}

		constexpr inline auto IoPortWrite(std::uint16_t port_v, std::uint8_t data_v) -> std::int32_t 
		{
			switch (port_v)
			{
				/***********************
				 *	RAM DAC
				 ***********************/
			case Port_DacPixelMask:
				ramdac.mask = data_v;
				return ERROR_SUCCESS;

			case Port_DacIndexWrite:
				ramdac.index = data_v * 3u;
				ramdac.latch = 0x3u;
				return ERROR_SUCCESS;

			case Port_DacIndexRead:
				ramdac.index = data_v * 3u;
				ramdac.latch = 0x0u;
				return ERROR_SUCCESS;

			case Port_DacDataWrite:
				ramdac.color[ramdac.index] = data_v & 0x3Fu;
				ramdac.index += 1u;
				while (ramdac.index >= 0x300u)
					ramdac.index -= 0x300u;
				return ERROR_SUCCESS;

				/***********************
				 *	CRT CONTROLLER
				 ***********************/
			case Port_MdaCrtIndex:
			case Port_VgaCrtIndex:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
				crtctrl.index = data_v & 0x1Fu;
				while (crtctrl.index >= std::size(crtctrl.data))
					crtctrl.index -= std::size(crtctrl.data);
				return ERROR_SUCCESS;

			case Port_MdaCrtData:
			case Port_VgaCrtData:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
				if (crtctrl.index < std::size(crtctrl.data)) {
					if (!(crtctrl.data[0x11u] & 0x80u) || crtctrl.index > 0x07u) {
						crtctrl.data[crtctrl.index] = data_v;
					}
				}
				crtctrl.index += 1u;
				while (crtctrl.index >= std::size(crtctrl.data))
					crtctrl.index -= std::size(crtctrl.data);
				return ERROR_SUCCESS;

				/***********************
				 *	SEQUENCER
				 ***********************/
			case Port_SequencerIndex:
				sequencer.index = data_v & 0x7u;
				while (sequencer.index >= std::size(sequencer.data))
					sequencer.index -= std::size(sequencer.data);
				return ERROR_SUCCESS;

			case Port_SequencerData:
				sequencer.data[sequencer.index] = data_v;
				sequencer.index += 1u;
				while (sequencer.index >= std::size(sequencer.data))
					sequencer.index -= std::size(sequencer.data);
				return ERROR_SUCCESS;

				/*********************************
				 *	GRAHPICS CONTROLLER
				 *********************************/
			case Port_GraphicsCtrlIndex:
				graphics.index = data_v & 0x0Fu;
				while (graphics.index >= std::size(graphics.data))
					graphics.index -= std::size(graphics.data);
				return ERROR_SUCCESS;
			case Port_GraphicsCtrlData:
				graphics.data[graphics.index] = data_v;
				graphics.index += 1u;
				while (graphics.index >= std::size(graphics.data))
					graphics.index -= std::size(graphics.data);
				return ERROR_SUCCESS;

				/***********************
				 *	ATTRIBUTE CONTROLLER
				 ***********************/
			case Port_Attribute0:
				if (!attrib.latch) {
					attrib.latch = !attrib.latch;
					attrib.index_and_pas = data_v & 0x3Fu;
				}
				else {
					attrib.latch = !attrib.latch;
					if (attrib.index < std::size(attrib.data))
						attrib.data[attrib.index] = data_v;
				}
				return ERROR_SUCCESS;

			case Port_Attribute1:
				return ERROR_SUCCESS;

				/*********************************
				 *	MISC OUTPUT & FEATURE CONTROL
				 *********************************/
			case Port_MiscOutputWrite:
				misc_output = data_v;
				return ERROR_SUCCESS;

			case Port_MdaFeatureControl:
			case Port_VgaFeatureControl:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
				feature_control = data_v;
				return ERROR_SUCCESS;

			default:
				break;
			}
			//__debugbreak();
			return ERROR_SUCCESS;
		}

		
		auto IoPortFetch(std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>;

		//auto GetValue(ValueIndex index_v) const->std::uint64_t;
		//auto SetValue(ValueIndex index_v, std::uint64_t value_v) -> void;

/*
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


		auto DisplayEnableSkew() const->uint8_t;
		auto HorizontalRetraceSkew() const->uint8_t;
		auto CursorSkew() const->uint8_t;

		auto CharacterWidth() const->uint8_t;
		auto CharacterHeight() const->uint8_t;

		auto ScreenDisable() const -> bool;

		auto ScanlineDouble() const -> bool;
		auto ScanlineClockDivide() const -> bool;

		auto MasterClockRate() const->uint64_t;
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
		auto MemoryMapSelect() const->utils::region32_type;
		auto CharsetA() const->utils::region32_type;
		auto CharsetB() const->utils::region32_type;
*/
		auto Log() const -> void;

		struct
		{
			uint8_t index;
			union
			{
				uint8_t data[0x19u];
				struct
				{
					// 0x00
					uint8_t horizontal_total_0_7:8;
					// 0x01
					uint8_t horizontal_display_end:8;
					// 0x02
					uint8_t horizontal_blanking_start:8;
					// 0x03
					uint8_t horizontal_blanking_end_0_4:5;
					uint8_t display_enable_skew:2;
					uint8_t enable_vrtical_retrace_access:1;
					// 0x04
					uint8_t horizontal_retrace_start:8;
					// 0x05
					uint8_t horizontal_retrace_end:5;
					uint8_t horizontal_retrace_skew:2;
					uint8_t horizontal_blanking_end_5:1;
					// 0x06
					uint8_t vertical_total_0_7:8;
					// 0x07
					uint8_t vertical_total_8:1;
					uint8_t vertical_display_end_8:1;
					uint8_t vertical_retrace_start_8:1;
					uint8_t vertical_blanking_start_8:1;
					uint8_t line_compare_8:1;
					uint8_t vertical_total_9:1;
					uint8_t vertical_display_end_9:1;
					uint8_t vertical_retrace_start_9:1;
					// 0x08
					uint8_t preset_row_scan:5;
					uint8_t byte_panning:2;
					uint8_t _0:1;
					// 0x09
					uint8_t maximum_scan_line:5;
					uint8_t vertical_blanking_start_9:1;
					uint8_t line_compare_9:1;
					uint8_t scan_doubling:1;
					// 0x0A
					uint8_t cursor_line_start:5;
					uint8_t cursor_disable:1;
					uint8_t _1:2;

					// 0x0B
					uint8_t cursor_line_end:5;
					uint8_t cursor_skew:2;
					uint8_t _2:1;
					// 0x0C
					uint8_t start_address_msb:8;
					// 0x0D
					uint8_t start_address_lsb:8;
					// 0x0E
					uint8_t cursor_location_msb:8;
					// 0x0F
					uint8_t cursor_location_lsb:8;
					// 0x10
					uint8_t vertical_retrace_start_0_7;
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
	/*
	inline VgaState::VgaState()
	{
		static_assert(std::is_trivially_copyable_v<VgaState>);
		std::memset(this, 0, sizeof(*this));
		misc_output = 0x03u;
	}
	*/



	inline auto VgaState::IoPortFetch(std::uint16_t port_v) -> std::tuple<std::int32_t, std::uint8_t>
	{
		uint8_t tmp_v{ 0 };
		switch (port_v)
		{
			/***********************
			 *	RAM DAC
			 *******************/
		case Port_DacPixelMask:
			return { ERROR_SUCCESS, ramdac.mask };

		case Port_DacDataRead:
			tmp_v = ramdac.color[ramdac.index];
			ramdac.index += 1u;
			while (ramdac.index >= 0x300u)
				ramdac.index -= 0x300u;
			return { ERROR_SUCCESS, tmp_v };

		case Port_DacStateRead:
			return { ERROR_SUCCESS, ramdac.latch };

			/***********************
			 *	CRT CONTROLLER
			 ***********************/
		case Port_MdaCrtIndex:
		case Port_VgaCrtIndex:
			if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
			return { ERROR_SUCCESS, crtctrl.index };

		case Port_VgaCrtData:
		case Port_MdaCrtData:
			if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
			tmp_v = crtctrl.data[crtctrl.index];
			crtctrl.index += 1u;
			while (crtctrl.index >= std::size(crtctrl.data))
				crtctrl.index -= std::size(crtctrl.data);
			return { ERROR_SUCCESS, tmp_v };

		case Port_MdaInputStatus:
		case Port_VgaInputStatus:
			if ((port_v < detail::port_vga_io(0x3D0u)) == bool(misc_output & 0x1u)) break;
			attrib.latch = false;
			return { ERROR_SUCCESS, 0 };

			/***********************
			 *	SEQUENCER
			 ***********************/
		case Port_SequencerIndex:
			return { ERROR_SUCCESS, sequencer.index };

		case Port_SequencerData:
			tmp_v = sequencer.data[sequencer.index];
			sequencer.index += 1u;
			while (sequencer.index >= std::size(sequencer.data))
				sequencer.index -= std::size(sequencer.data);
			return { ERROR_SUCCESS, tmp_v };

			/**************************
			 *	GRAPHICS CONTROLLER
			 **************************/
		case Port_GraphicsCtrlIndex:
			return { ERROR_SUCCESS, graphics.index };

		case Port_GraphicsCtrlData:
			tmp_v = graphics.data[graphics.index];
			graphics.index += 1u;
			while (graphics.index >= std::size(graphics.data))
				graphics.index -= std::size(graphics.data);
			return { ERROR_SUCCESS, tmp_v };

			/***********************
			 *	ATTRIBUTE CONTROLLER
			 ***********************/
		case Port_Attribute0:
			return { ERROR_SUCCESS, attrib.index_and_pas };

		case Port_Attribute1:
			if (attrib.index < std::size(attrib.data))
				return { ERROR_SUCCESS, attrib.data[attrib.index] };
			return { ERROR_SUCCESS, 0x00u };

			/*********************************
			 *	MISC OUTPUT & FEATURE CONTROL
			 *********************************/
		case Port_MiscOutputRead:
			return { ERROR_SUCCESS, misc_output };

		case Port_FeatureControlRead:
			return { ERROR_SUCCESS, feature_control };

		case Port_InputStatus:
			return { ERROR_SUCCESS, 0x00u };

		default:
			break;
		}

		//__debugbreak();
		return { ERROR_SUCCESS, 0xffu };
	}

	/*
	inline auto VgaState::CharacterWidth() const -> uint8_t
	{
		return sequencer.clocking_mode & 0x1u ? 8u : 9u;
	}

	inline auto VgaState::CharacterHeight() const -> uint8_t
	{
		return (crtctrl.maximum_scan_line & 0x1F) + 1u;
	}

	inline auto VgaState::ScanlineDouble() const -> bool
	{
		return bool(crtctrl.maximum_scan_line & 0x80u);
	}

	inline auto VgaState::ScanlineClockDivide() const -> bool
	{
		return bool(crtctrl.crt_mode_control & 0x04u);
	}

	inline auto VgaState::MemoryClockDivide() const -> bool
	{
		return bool(crtctrl.crt_mode_control & 0x08u);
	}

	inline auto VgaState::ScreenDisable() const -> bool
	{
		return bool(sequencer.clocking_mode & 0x20u);
	}

	inline auto VgaState::ShiftFour() const -> bool
	{
		return bool(sequencer.clocking_mode & 0x10u);
	}

	inline auto VgaState::DotClockDivide() const -> bool
	{
		return bool(sequencer.clocking_mode & 0x08u);
	}

	inline auto VgaState::ShiftLoadRate() const -> bool
	{
		return bool(sequencer.clocking_mode & 0x04u);
	}

	inline auto VgaState::MasterClockDivide() const -> bool
	{
		return bool(sequencer.character_map_select & 0x08u);
	}

	inline auto VgaState::ByteAddressMode() const -> bool
	{
		return bool(crtctrl.crt_mode_control & 0x40u);
	}

	inline auto VgaState::OddEventDisable() const -> bool
	{
		return bool(sequencer.memory_mode & 0x04u);
	}

	inline auto VgaState::ChainOddEven() const -> bool
	{
		return bool(graphics.miscellaneous & 0x02u);
	}

	inline auto VgaState::ChainFour() const -> bool
	{
		return bool(sequencer.memory_mode & 0x08u);
	}

	inline auto VgaState::GraphicsMode() const -> bool
	{
		return bool(graphics.miscellaneous & 0x01u);
	}

	inline auto VgaState::MemoryMapSelect() const -> utils::region32_type
	{
		switch ((graphics.miscellaneous >> 2u) & 0x3u)
		{
		case 0x0: return { 0xA0000u, 0x20000u };
		case 0x1: return { 0xA0000u, 0x10000u };
		case 0x2: return { 0xB0000u, 0x08000u };
		default:
		case 0x3: return { 0xB8000u, 0x08000u };
		}

	}

	inline auto VgaState::CharsetA() const -> utils::region32_type
	{
		const uint8_t value_v
		{ ((sequencer.character_map_select >> 2u) & 0x3u)
		+ ((sequencer.character_map_select >> 5u) & 0x1u) };

		switch (value_v & 0x7u)
		{
		default:
		case 0b000: return { 0x0000u, 0x2000u };
		case 0b001: return { 0x4000u, 0x2000u };
		case 0b010: return { 0x8000u, 0x2000u };
		case 0b011: return { 0xC000u, 0x2000u };
		case 0b100: return { 0x2000u, 0x2000u };
		case 0b101: return { 0x6000u, 0x2000u };
		case 0b110: return { 0xA000u, 0x2000u };
		case 0b111: return { 0xE000u, 0x2000u };
		}
	}

	inline auto VgaState::CharsetB() const -> utils::region32_type
	{
		const uint8_t value_v
		{ ((sequencer.character_map_select >> 0u) & 0x3u)
		+ ((sequencer.character_map_select >> 4u) & 0x1u) };

		switch (value_v & 0x7u)
		{
		default:
		case 0b000: return { 0x0000u, 0x2000u };
		case 0b001: return { 0x4000u, 0x2000u };
		case 0b010: return { 0x8000u, 0x2000u };
		case 0b011: return { 0xC000u, 0x2000u };
		case 0b100: return { 0x2000u, 0x2000u };
		case 0b101: return { 0x6000u, 0x2000u };
		case 0b110: return { 0xA000u, 0x2000u };
		case 0b111: return { 0xE000u, 0x2000u };
		}
	}

	inline auto VgaState::MasterClockRate() const -> uint64_t
	{
		switch ((misc_output & 0xCu) >> 2u)
		{
		default:
		case 0x0: return 25175000ull;
		case 0x1: return 28322000ull;
		case 0x2: return 31500000ull;
		case 0x3: return 40000000ull;
		}
	}

	inline auto VgaState::HorizontalTotal() const -> uint16_t
	{
		return CharacterWidth() * (crtctrl.horizontal_total + 5u);
	}

	inline auto VgaState::HorizontalDisplayEnd() const -> uint16_t
	{
		return CharacterWidth() * (crtctrl.horizontal_display_end + 1u);
	}

	inline auto VgaState::HorizontalRetraceStart() const -> uint16_t
	{
		return CharacterWidth() * crtctrl.horizontal_retrace_start;
	}

	inline auto VgaState::HorizontalRetraceEnd() const -> uint16_t
	{
		auto const lsb_v = crtctrl.horizontal_retrace_end & 0x1Fu;
		auto const counter_v = lsb_v + (HorizontalRetraceStart() & ~0x1Fu);
		if (counter_v > HorizontalTotal())
			return counter_v;
		return lsb_v;
	}

	inline auto VgaState::HorizontalBlankingStart() const -> uint16_t
	{
		return CharacterWidth() * crtctrl.horizontal_blanking_start;
	}

	inline auto VgaState::HorizontalBlankingEnd() const -> uint16_t
	{
		auto const lsb_v = (
			((crtctrl.horizontal_blanking_end & 0x1Fu) >> 0u) +
			((crtctrl.horizontal_retrace_end & 0x80u) >> 2u));
		auto const counter_v = lsb_v + (HorizontalBlankingStart() & ~0x1Fu);
		if (counter_v > HorizontalTotal())
			return counter_v;
		return lsb_v;
	}

	inline auto VgaState::VerticalTotal() const -> uint16_t
	{
		return crtctrl.vertical_total
			+ 0x100u * (crtctrl.overflow & 0x01u)
			+ 0x010u * (crtctrl.overflow & 0x20u)
			;
	}

	inline auto VgaState::VerticalDisplayEnd() const -> uint16_t
	{
		return crtctrl.vertical_blanking_end
			+ 0x80u * (crtctrl.overflow & 0x02u)
			+ 0x08u * (crtctrl.overflow & 0x40u)
			;
	}

	inline auto VgaState::VerticalRetraceStart() const -> uint16_t
	{
		return crtctrl.vertical_retrace_start
			+ 0x40u * (crtctrl.overflow & 0x04u)
			+ 0x04u * (crtctrl.overflow & 0x80u)
			;
	}

	inline auto VgaState::VerticalRetraceEnd() const -> uint16_t
	{
		auto const lsb_v = crtctrl.vertical_retrace_end & 0x0Fu;
		auto counter_v = lsb_v + (VerticalRetraceStart() & ~0x0Fu);
		if (counter_v > VerticalTotal())
			return counter_v;
		return lsb_v;
	}

	inline auto VgaState::VerticalBlankingStart() const -> uint16_t
	{
		return crtctrl.vertical_blanking_start
			+ (crtctrl.maximum_scan_line & 0x20u) * 0x10u
			+ (crtctrl.overflow & 0x08u) * 0x20u
			;
	}

	inline auto VgaState::VerticalBlankingEnd() const -> uint16_t
	{
		auto const lsb_v = crtctrl.vertical_blanking_end & 0x7Fu;
		auto counter_v = lsb_v + (VerticalBlankingStart() & ~0x7Fu);
		if (counter_v > VerticalTotal())
			return counter_v;
		return lsb_v;
	}

	inline auto VgaState::DisplayEnableSkew() const -> uint8_t
	{
		return (crtctrl.horizontal_blanking_end >> 5u) & 3u;
	}

	inline auto VgaState::HorizontalRetraceSkew() const -> uint8_t
	{
		return (crtctrl.horizontal_retrace_end >> 5u) & 3u;
	}

	inline auto VgaState::CursorSkew() const -> uint8_t
	{
		return (crtctrl.cursor_end >> 5u) & 3u;
	}
	*/

	inline auto VgaState::Log() const -> void
	{
		/*
		using namespace std::string_literals;
		using utils::logger;

		std::string color_tbl;

		static constexpr const auto draw_rgb = [](auto&& r, auto&& g, auto&& b) {
			return std::format("\x1b[48;2;{};{};{}m  \x1b[0m", r, g, b);
			};
		static constexpr const auto draw_index_rgb = [](auto&& i, auto&& table) {
			return draw_rgb(
				table[3u * i + 0u] * 4,
				table[3u * i + 1u] * 4,
				table[3u * i + 2u] * 4
			);
			};

		for (auto j = 0u; j < 0x10u; ++j)
		{
			color_tbl.append("\n  > ");
			for (auto i = 0u; i < 0x10u; ++i)
			{
				auto r = ramdac.color[3u * (j * 0x10u + i) + 0u];
				auto g = ramdac.color[3u * (j * 0x10u + i) + 1u];
				auto b = ramdac.color[3u * (j * 0x10u + i) + 2u];
				color_tbl.append(draw_rgb(r * 0x4, g * 0x4, b * 0x4));
			}
		}

#define Fmt(X)     std::format("  > {:.<35} = {}\n"      , #X, X)
#define Fmt_(X, Y) std::format("  > {:.<35} = {} ({})\n" , #X, X, Y)
#define FmtH(X)    std::format("  > {:.<35} = 0x{:02X}\n", #X, X)
		logger::debug(logger::deflog, "video state : \n{}\n",
			std::string()

			+ Fmt(HorizontalTotal())
			+ Fmt(HorizontalDisplayEnd())
			+ Fmt(HorizontalBlankingStart())
			+ Fmt(HorizontalRetraceStart())
			+ Fmt(HorizontalRetraceEnd())
			+ Fmt(HorizontalBlankingEnd())

			+ Fmt(VerticalTotal())
			+ Fmt(VerticalDisplayEnd())
			+ Fmt(VerticalBlankingStart())
			+ Fmt(VerticalRetraceStart())
			+ Fmt(VerticalRetraceEnd())
			+ Fmt(VerticalBlankingEnd())

			+ Fmt(DisplayEnableSkew())
			+ Fmt(HorizontalRetraceSkew())
			+ Fmt(CursorSkew())

			+ Fmt(ScreenDisable())

			+ Fmt(MasterClockRate() * 1e-6)
			+ Fmt(MemoryClockDivide())
			+ Fmt(DotClockDivide())

			+ Fmt(ScanlineDouble())
			+ Fmt(ScanlineClockDivide())

			+ Fmt(ShiftLoadRate())
			+ Fmt(ShiftFour())
			+ Fmt(ChainFour())
			+ Fmt(ByteAddressMode())
			+ Fmt(OddEventDisable())

			+ Fmt(ChainOddEven())
			+ Fmt(GraphicsMode())
			+ FmtH(MemoryMapSelect().base())
			+ FmtH(MemoryMapSelect().end())
			+ FmtH(CharsetA().base())
			+ FmtH(CharsetA().end())
			+ FmtH(CharsetB().base())
			+ FmtH(CharsetB().end())

			+ Fmt(CharacterWidth())
			+ Fmt(CharacterHeight())

			+ FmtH(crtctrl.horizontal_total)
			+ FmtH(crtctrl.horizontal_display_end)
			+ FmtH(crtctrl.horizontal_blanking_start)
			+ FmtH(crtctrl.horizontal_blanking_end)
			+ FmtH(crtctrl.horizontal_retrace_start)
			+ FmtH(crtctrl.horizontal_retrace_end)
			+ FmtH(crtctrl.vertical_total)
			+ FmtH(crtctrl.overflow)
			+ FmtH(crtctrl.preset_row_scan)
			+ FmtH(crtctrl.maximum_scan_line)
			+ FmtH(crtctrl.cursor_start)
			+ FmtH(crtctrl.cursor_end)
			+ FmtH(crtctrl.start_address_high)
			+ FmtH(crtctrl.start_address_low)
			+ FmtH(crtctrl.cursor_location_high)
			+ FmtH(crtctrl.cursor_location_low)
			+ FmtH(crtctrl.vertical_retrace_start)
			+ FmtH(crtctrl.vertical_retrace_end)
			+ FmtH(crtctrl.vertical_display_end)
			+ FmtH(crtctrl.offset)
			+ FmtH(crtctrl.underline_location)
			+ FmtH(crtctrl.vertical_blanking_start)
			+ FmtH(crtctrl.vertical_blanking_end)
			+ FmtH(crtctrl.crt_mode_control)
			+ FmtH(crtctrl.line_compare)
			+ FmtH(sequencer.reset)
			+ FmtH(sequencer.clocking_mode)
			+ FmtH(sequencer.map_mask)
			+ FmtH(sequencer.character_map_select)
			+ FmtH(sequencer.memory_mode)
			+ FmtH(graphics.set_or_reset)
			+ FmtH(graphics.enable_set_or_reset)
			+ FmtH(graphics.color_compare)
			+ FmtH(graphics.data_rotate)
			+ FmtH(graphics.read_map_select)
			+ FmtH(graphics.graphics_mode)
			+ FmtH(graphics.miscellaneous)
			+ FmtH(graphics.color_dont_care)
			+ FmtH(graphics.bit_mask)
			+ Fmt_(attrib.palette[0x0], draw_index_rgb(attrib.palette[0x0], ramdac.color))
			+ Fmt_(attrib.palette[0x1], draw_index_rgb(attrib.palette[0x1], ramdac.color))
			+ Fmt_(attrib.palette[0x2], draw_index_rgb(attrib.palette[0x2], ramdac.color))
			+ Fmt_(attrib.palette[0x3], draw_index_rgb(attrib.palette[0x3], ramdac.color))
			+ Fmt_(attrib.palette[0x4], draw_index_rgb(attrib.palette[0x4], ramdac.color))
			+ Fmt_(attrib.palette[0x5], draw_index_rgb(attrib.palette[0x5], ramdac.color))
			+ Fmt_(attrib.palette[0x6], draw_index_rgb(attrib.palette[0x6], ramdac.color))
			+ Fmt_(attrib.palette[0x7], draw_index_rgb(attrib.palette[0x7], ramdac.color))
			+ Fmt_(attrib.palette[0x8], draw_index_rgb(attrib.palette[0x8], ramdac.color))
			+ Fmt_(attrib.palette[0x9], draw_index_rgb(attrib.palette[0x9], ramdac.color))
			+ Fmt_(attrib.palette[0xA], draw_index_rgb(attrib.palette[0xA], ramdac.color))
			+ Fmt_(attrib.palette[0xB], draw_index_rgb(attrib.palette[0xB], ramdac.color))
			+ Fmt_(attrib.palette[0xC], draw_index_rgb(attrib.palette[0xC], ramdac.color))
			+ Fmt_(attrib.palette[0xD], draw_index_rgb(attrib.palette[0xD], ramdac.color))
			+ Fmt_(attrib.palette[0xE], draw_index_rgb(attrib.palette[0xE], ramdac.color))
			+ Fmt_(attrib.palette[0xF], draw_index_rgb(attrib.palette[0xF], ramdac.color))
			+ FmtH(attrib.mode_control)
			+ FmtH(attrib.overscan_color)
			+ FmtH(attrib.color_plane_enable)
			+ FmtH(attrib.horizontal_panning)
			+ FmtH(attrib.color_select)
			+ FmtH(ramdac.latch)
			+ FmtH(ramdac.mask)
			+ "  > ramdac.color => "s + color_tbl + "\n");
#undef Fmt
#undef Fmt_
#undef FmtH
		*/
	}

}