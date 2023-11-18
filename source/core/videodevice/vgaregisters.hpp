#pragma once

#include <cstdint>
#include <cstddef>

#include <type_traits>

#include <utils/logger.hpp>

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

	static inline constexpr const auto SyncPolarity_350					= 0b10u;	
	static inline constexpr const auto SyncPolarity_400					= 0b01u;
	static inline constexpr const auto SyncPolarity_480					= 0b11u;

	static inline constexpr const auto ClockSelect_25MHz				= 0b00u;
	static inline constexpr const auto ClockSelect_28MHz				= 0b01u;
	// Custom
	static inline constexpr const auto ClockSelect_31MHz				= 0b10u;
	static inline constexpr const auto ClockSelect_40MHz				= 0b11u;


#pragma pack(push, 1)
	struct VGARegisters
	{
		enum class ValueIndex : std::uint16_t {

			DrvHorizontalTotalChars,
			DrvHorizontalVisibleChars,
			DrvHorizontalTotal,
			DrvHorizontalVisible,
      DrvHorizontalBlankingStartChars,
      DrvHorizontalBlankingStart,
      DrvHorizontalBlankingEndChars,
      DrvHorizontalBlankingEnd,
      DrvHorizontalRetraceStartChars,
      DrvHorizontalRetraceStart,
      DrvHorizontalRetraceEndChars,
      DrvHorizontalRetraceEnd,

			DrvVerticalTotal,
			DrvVerticalVisible,
			DrvVerticalBlankingStart,
			DrvVerticalBlankingEnd,
			DrvVerticalRetraceStart,
			DrvVerticalRetraceEnd,

			DrvCharacterWidth,
			DrvCharacterHeight,

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
			VerticalBlankingEnd,

      MasterClockSelect,
			MaximumScanline,
			DotClockRate,
			EightDotMode,
      DoubleWordAddressing,
      AddressClockDivideByFour,
      AddressClockDivideByTwo,
      ScanlineClockDivideByTwo, 
      DoubleScanning
		};

		constexpr inline VGARegisters() noexcept 
		{
			for (auto& value_v: crtctrl.data  ) value_v = 0x00u;
			for (auto& value_v: sequencer.data) value_v = 0x00u;
			for (auto& value_v: graphics.data ) value_v = 0x00u;			
			for (auto& value_v: attrib.data   ) value_v = 0x00u;
      for (auto& value_v: ramdac.color  ) value_v = 0x00u;

      crtctrl.index                   = 0x00u;
      sequencer.index                 = 0x00u;
      graphics.index                  = 0x00u;
      attrib.index_and_pas            = 0x00u;
      ramdac.latch                    = 0x00u;
			ramdac.index                    = 0x00u;
			ramdac.flags                    = 0x00u;
			ramdac.mask                     = 0x00u;
			miscellanious.value             = 0x00u;
			miscellanious.io_address_select = 0x01u;
			miscellanious.ram_access_enable = 0x01u;
			miscellanious.sync_polarity     = SyncPolarity_400;
			miscellanious.clock_select      = ClockSelect_28MHz;

			feature_control = 0x00u;
		}

		constexpr inline auto IoPortWrite(std::uint16_t port_v, std::uint8_t data_v) noexcept -> void 
		{
			switch (port_v)
			{
				/***********************
				 *	RAM DAC
				 ***********************/
			case Port_DacPixelMask:
				ramdac.mask = data_v;
				return;

			case Port_DacIndexWrite:
				ramdac.index = data_v * 3u;
				ramdac.latch = 0x3u;
				return;

			case Port_DacIndexRead:
				ramdac.index = data_v * 3u;
				ramdac.latch = 0x0u;
				return;

			case Port_DacDataWrite:
				ramdac.color[ramdac.index] = data_v & 0x3Fu;
				ramdac.index += 1u;
				while (ramdac.index >= 0x300u)
					ramdac.index -= 0x300u;
				return;

				/***********************
				 *	CRT CONTROLLER
				 ***********************/
			case Port_MdaCrtIndex:
			case Port_VgaCrtIndex:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
				crtctrl.index = data_v & 0x1Fu;
				return;

			case Port_MdaCrtData:
			case Port_VgaCrtData:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
				if (crtctrl.index < std::size(crtctrl.data)) {
					if (!(crtctrl.data[0x11u] & 0x80u) || crtctrl.index > 0x07u) {
						crtctrl.data[crtctrl.index] = data_v;
					}
				}
				return;

				/***********************
				 *	SEQUENCER
				 ***********************/
			case Port_SequencerIndex:
				sequencer.index = data_v & 0x7u;
				return;

			case Port_SequencerData:
        if (sequencer.index < std::size(sequencer.data))
				  sequencer.data[sequencer.index] = data_v;
				return;

				/*********************************
				 *	GRAHPICS CONTROLLER
				 *********************************/
			case Port_GraphicsCtrlIndex:
				graphics.index = data_v & 0x0Fu;
				return;
			case Port_GraphicsCtrlData:
        if (graphics.index < std::size(graphics.data))
				  graphics.data[graphics.index] = data_v;
				return;

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
				return ;

			case Port_Attribute1:
				return ;

				/*********************************
				 *	MISC OUTPUT & FEATURE CONTROL
				 *********************************/
			case Port_MiscOutputWrite:
				miscellanious.value = data_v;
				return ;

			case Port_MdaFeatureControl:
			case Port_VgaFeatureControl:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
				feature_control = data_v;
				return ;

			default:
				break;
			}
			
			return ;
		}

		
		constexpr inline auto IoPortFetch(std::uint16_t port_v) noexcept -> std::uint8_t
		{
			uint8_t tmp_v{ 0 };
			switch (port_v)
			{
				/***********************
				 *	RAM DAC
				 *******************/
			case Port_DacPixelMask:
				return ramdac.mask;

			case Port_DacDataRead:
				tmp_v = ramdac.color[ramdac.index];
				ramdac.index += 1u;
				while (ramdac.index >= 0x300u)
					ramdac.index -= 0x300u;
				return tmp_v;

			case Port_DacStateRead:
				return ramdac.latch;

				/***********************
				 *	CRT CONTROLLER
				 ***********************/
			case Port_MdaCrtIndex:
			case Port_VgaCrtIndex:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
				return crtctrl.index;

			case Port_VgaCrtData:
			case Port_MdaCrtData:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
        tmp_v = 0xffu;
        if (crtctrl.index < std::size(crtctrl.data))
				  tmp_v = crtctrl.data[crtctrl.index];  
				return tmp_v;

			case Port_MdaInputStatus:
			case Port_VgaInputStatus:
				if ((port_v < detail::port_vga_io(0x3D0u)) == bool(miscellanious.io_address_select)) break;
				attrib.latch = false;
				return 0x00u;

				/***********************
				 *	SEQUENCER
				 ***********************/
			case Port_SequencerIndex:
				return sequencer.index;

			case Port_SequencerData:
        tmp_v = 0xffu;
        if (sequencer.index < std::size(sequencer.data))
				  tmp_v = sequencer.data[sequencer.index];
				return tmp_v;

				/**************************
				 *	GRAPHICS CONTROLLER
				 **************************/
			case Port_GraphicsCtrlIndex:
				return graphics.index;

			case Port_GraphicsCtrlData:
        tmp_v = 0xffu;
        if (graphics.index < std::size(graphics.data))  
          tmp_v = graphics.data[graphics.index];
				return tmp_v;

				/***********************
				 *	ATTRIBUTE CONTROLLER
				 ***********************/
			case Port_Attribute0:
				return attrib.index_and_pas;

			case Port_Attribute1:
				if (attrib.index < std::size(attrib.data))
					return attrib.data[attrib.index];
				return 0xffu;

				/*********************************
				 *	MISC OUTPUT & FEATURE CONTROL
				 *********************************/
			case Port_MiscOutputRead:
				return miscellanious.value;

			case Port_FeatureControlRead:
				return feature_control;

			case Port_InputStatus:
				return 0x00u;

			default:
				break;
			}

			return 0xffu;
		}

		template <ValueIndex _Index>
		constexpr inline auto GetValue() const
		{
			static constexpr auto F = [](auto x, auto m) { x &= (m - 1); return !x ? m : x; };

			using enum ValueIndex;
			     if constexpr (_Index == EightDotMode              ) return sequencer.eight_dot_mode ;
			else if constexpr (_Index == DotClockRate              ) return sequencer.dot_clock_rate ;
      else if constexpr (_Index == MasterClockSelect         ) return miscellanious.clock_select;
      else if constexpr (_Index == MaximumScanline           ) return crtctrl.maximum_scan_line ;
			else if constexpr (_Index == HorizontalTotal           ) return crtctrl.horizontal_total ;
			else if constexpr (_Index == HorizontalDisplayEnd      ) return crtctrl.horizontal_display_end ;
			else if constexpr (_Index == HorizontalRetraceStart    ) return crtctrl.horizontal_retrace_start ;      
			else if constexpr (_Index == HorizontalRetraceEnd      ) return crtctrl.horizontal_retrace_end ;
			else if constexpr (_Index == HorizontalBlankingStart   ) return crtctrl.horizontal_blanking_start ;     
			else if constexpr (_Index == HorizontalBlankingEnd     ) return crtctrl.horizontal_blanking_end_0_4
						                                                        + crtctrl.horizontal_blanking_end_5 ;			
			else if constexpr (_Index == VerticalTotal             ) return crtctrl.vertical_total_0_7 * 0x1u
						                                                        + crtctrl.vertical_total_8 * 0x100u 
						                                                        + crtctrl.vertical_total_9 * 0x200u ;
			else if constexpr (_Index == VerticalDisplayEnd        ) return crtctrl.vertical_display_end_0_7 * 0x1u
						                                                        + crtctrl.vertical_display_end_8 * 0x100u 
					                                                          + crtctrl.vertical_display_end_9 * 0x200u ;		
			else if constexpr (_Index == VerticalRetraceStart      ) return crtctrl.vertical_retrace_start_0_7 * 0x1u
				 	                                                          + crtctrl.vertical_retrace_start_8 * 0x100u 
				 	                                                          + crtctrl.vertical_retrace_start_9 * 0x200u ;			
			else if constexpr (_Index == VerticalRetraceEnd        ) return crtctrl.vertical_retrace_end ;
			else if constexpr (_Index == VerticalBlankingStart     ) return crtctrl.vertical_blanking_start_0_7 * 0x1u
					                                                          + crtctrl.vertical_blanking_start_8 * 0x100u 
					                                                          + crtctrl.vertical_blanking_start_9 * 0x200u ;		
			else if constexpr (_Index == VerticalBlankingEnd       ) return crtctrl.vertical_blanking_end ;	

			else if constexpr (_Index == DrvHorizontalTotalChars   ) return (GetValue<HorizontalTotal>() + 5u) << GetValue<DotClockRate>();
      else if constexpr (_Index == DrvHorizontalVisibleChars ) return (GetValue<HorizontalDisplayEnd>() + 1u) << GetValue<DotClockRate>();
      else if constexpr (_Index == DrvVerticalTotal          ) return GetValue<VerticalTotal>() + 2u;
      else if constexpr (_Index == DrvVerticalVisible        ) return GetValue<VerticalDisplayEnd>() + 1u;
      else if constexpr (_Index == DrvCharacterWidth         ) return 8u+!GetValue<EightDotMode>();
      else if constexpr (_Index == DrvCharacterHeight        ) return GetValue<MaximumScanline>() + 1u;
      else if constexpr (_Index == DrvHorizontalTotal        ) return GetValue<DrvCharacterWidth>() * GetValue<DrvHorizontalTotalChars>();
      else if constexpr (_Index == DrvHorizontalVisible      ) return GetValue<DrvCharacterWidth>() * GetValue<DrvHorizontalVisibleChars>();

      else {
        //static_assert(sizeof(_Index)==0, "invalid value index"); 
        return 0u;
      }
			
		}

		auto Log() const -> void
		{
			using namespace std::string_literals;
			using utils::logger;

		#define Fmt(X)  std::format("  > {:.<35}: {}\n", #X, GetValue<ValueIndex::X>())
			logger::debug(logger::deflog, "video state : \n{}\n",
				std::string()

        
        + Fmt(DrvHorizontalTotalChars)
				+ Fmt(DrvHorizontalVisibleChars)
        + Fmt(DrvHorizontalTotal)
        + Fmt(DrvHorizontalVisible)
				+ Fmt(DrvVerticalTotal)
				+ Fmt(DrvVerticalVisible)
				+ Fmt(DrvCharacterWidth)
				+ Fmt(DrvCharacterHeight)

				+ Fmt(HorizontalDisplayEnd)
				+ Fmt(HorizontalRetraceStart)
				+ Fmt(HorizontalBlankingStart)
				+ Fmt(HorizontalBlankingEnd)
				+ Fmt(HorizontalRetraceEnd)
				+ Fmt(HorizontalTotal)
				+ Fmt(VerticalTotal)
				+ Fmt(VerticalDisplayEnd)
				+ Fmt(VerticalRetraceStart)
				+ Fmt(VerticalBlankingStart)
				+ Fmt(VerticalBlankingEnd)
				+ Fmt(VerticalRetraceEnd)
        + Fmt(MaximumScanline)
        + Fmt(DotClockRate) 
        + Fmt(EightDotMode)

			);
		#undef Fmt
		}

		struct
		{
			uint8_t index;
			union
			{
				uint8_t data[0x19u];
				struct
				{
					// 0x00
					uint8_t horizontal_total:8;
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
					uint8_t vertical_retrace_start_0_7:8;
					// 0x11
					uint8_t vertical_retrace_end:4;
					uint8_t _3:2;
					uint8_t bandwidth:1;
					uint8_t protect:1;
					// 0x12
					uint8_t vertical_display_end_0_7:8;
					// 0x13
					uint8_t offset:8;
					// 0x14
					uint8_t underline_location:5;
					uint8_t memory_address_clock_divide_by_four:1;
					uint8_t double_word_addressing:1;
					uint8_t _4:1;
					// 0x15
					uint8_t vertical_blanking_start_0_7:8;
					// 0x16
					uint8_t vertical_blanking_end:7;
					uint8_t _5:1;
					// 0x17
					uint8_t map_display_address_13:1;
					uint8_t map_display_address_14:1;
					uint8_t scanline_clock_divide_by_two:1;
					uint8_t memory_address_clock_divide_by_two:1;
					uint8_t _6:1;
					uint8_t address_wrap_select:1;
					uint8_t word_byte_mode_select:1;
					uint8_t sync_enable:1;					
					// 0x18
					uint8_t line_compare_0_7:8;
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
					// 0x00
					uint8_t synchroneous_reset:1;
					uint8_t asynchroneous_reset:1;
					uint8_t _0:6;
					// 0x01
					uint8_t eight_dot_mode:1;
					uint8_t _1:1;
					uint8_t shift_load_rate:1;
					uint8_t dot_clock_rate:1;
					uint8_t shift_four_enable:1;
					uint8_t screen_disable:1;
					uint8_t _2:2;
					// 0x02
					uint8_t memory_plane_write_mask:4;
					uint8_t _3:4;
					// 0x03
					uint8_t character_map_select_b_0_1:2;
					uint8_t character_map_select_a_0_1:2;
					uint8_t character_map_select_b_2:1;
					uint8_t character_map_select_a_2:1;
					uint8_t _4:2;
					// 0x04
					uint8_t _5:1;
					uint8_t extended_memory_enable:1;
					uint8_t host_odd_even_write_addressing_disable:1;
					uint8_t chain_four_enable:1;
					uint8_t _6:4;
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
					// 0x00
					uint8_t set_or_reset:4;
					uint8_t _0:4;
					// 0x01
					uint8_t enable_set_or_reset:4;
					uint8_t _1:4;
					// 0x02
					uint8_t color_compare:4;
					uint8_t _2:4;
					// 0x03
					uint8_t rotate_count:3;
					uint8_t logical_operation:2;
					uint8_t _3:3;
					// 0x04
					uint8_t read_map_select:2;
					uint8_t _4:6;
					// 0x05
					uint8_t write_mode:2;
					uint8_t _5:1;
					uint8_t read_mode:1;
					uint8_t host_odd_even_read_addressing_enable:1;
					uint8_t shift_register_interlieve_mode:1;
					uint8_t shift_256_color_mode:1;
					uint8_t _6:1;
					// 0x06
					uint8_t alphanumeric_mode_disable:1;
					uint8_t chain_odd_even_enable:1;
					uint8_t memory_map_select:2;
					uint8_t _7:4;
					// 0x07
					uint8_t color_dont_care:4;
					uint8_t _8:4;
					// 0x08
					uint8_t bit_mask;
				};
			};

		} graphics;

		struct
		{
			uint16_t index:16;
			uint8_t mask:8;
			uint8_t latch:8;
			uint8_t flags:8;
			uint8_t color[256u * 3u];
		} ramdac;

		struct
		{
			bool latch;

			union
			{
				struct {
					uint8_t index:5;
					uint8_t pas:1;
					uint8_t _4:2;
				};
				uint8_t index_and_pas;
			};
			union
			{
				uint8_t data[0x15u];
				struct {
					// 0x00-0x0F
					uint8_t palette[0x10u];
					// 0x10
					uint8_t graphics_enable:1;
					uint8_t monochome_emulation:1;
					uint8_t line_graphics_enable:1;
					uint8_t blink_enable:1;
					uint8_t _0:1;
					uint8_t pixel_panning_mode:1;
					uint8_t eight_bit_color_enable:1;
					uint8_t palette_bits_5_4_select:1;
					// 0x11
					uint8_t overscan_color;
					// 0x12
					uint8_t color_plane_enable:4;
					uint8_t _1:4;
					// 0x13
					uint8_t horizontal_panning:4;
					uint8_t _2:4;
					// 0x14
					uint8_t color_select_5_4:2;
					uint8_t color_select_7_6:2;
					uint8_t _3:4;
				};
			};
		} attrib;
		union {
			uint8_t value:8;
			struct {				
				uint8_t io_address_select:1;
				uint8_t ram_access_enable:1;
				uint8_t clock_select:2;
				uint8_t _0:1;
				uint8_t odd_even_page_select:1;
				uint8_t sync_polarity:2;
			};
		} miscellanious;
		uint8_t feature_control:8;

	};
#pragma pack(pop)	
}

static_assert(sizeof(core::VGARegisters) == 0x348u);