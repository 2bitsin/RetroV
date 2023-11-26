#pragma once

#include <cstdint>
#include <cstddef>

#include <type_traits>

#include <utils/logger.hpp>

#include <core/videodevice/vgaioconsts.hpp>

namespace core::videodevice
{

#pragma pack(push, 1)
	struct alignas(0x400u) VGARegisters
	{
		enum class ValueIndex : uint16_t {

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
      ReadModeSelect,
      WriteModeSelect,
      //DoubleWordAddressing,
      //AddressClockDivideByFour,
      //AddressClockDivideByTwo,
      //ScanlineClockDivideByTwo, 
      //DoubleScanning,
      ChainFourEnable,
      ReadMapSelect,

		};

		constexpr inline VGARegisters() noexcept 
		{
			for (auto& value_v: CRTC.data  ) value_v = 0x00u;
			for (auto& value_v: SEQU.data) value_v = 0x00u;
			for (auto& value_v: GFXC.data ) value_v = 0x00u;			
			for (auto& value_v: attrib.data   ) value_v = 0x00u;
      for (auto& value_v: RDAC.color  ) value_v = 0x00u;

      CRTC.index          = 0x00u;
      SEQU.index        = 0x00u;
      GFXC.index         = 0x00u;
      attrib.index_and_pas   = 0x00u;
      RDAC.latch           = 0x00u;
			RDAC.index           = 0x00u;
			RDAC.flags           = 0x00u;
			RDAC.mask            = 0x00u;
			MISC.value             = 0x00u;
			MISC.io_addr_3dx       = 0x01u;
			MISC.ram_access_enable = 0x01u;
			MISC.sync_polarity     = SyncPolarity_400;
			MISC.clock_select      = ClockSelect_28MHz;

			feature_control = 0x00u;
		}

		constexpr inline auto IoPortWrite(uint16_t port_v, uint8_t data_v) noexcept -> void 
		{
      auto const port_crt_data  = MISC.io_addr_3dx 
                                ? Port_VgaCrtData        
                                : Port_MdaCrtData ;
      auto const port_crt_index = MISC.io_addr_3dx 
                                ? Port_VgaCrtIndex       
                                : Port_MdaCrtIndex ;
      auto const port_status    = MISC.io_addr_3dx 
                                ? Port_VgaInputStatus
                                : Port_MdaInputStatus ;
      auto const port_control   = MISC.io_addr_3dx 
                                ? Port_VgaFeatureControl 
                                : Port_MdaFeatureControl ;

			switch (port_v)
			{
				/***********************
				 *	RAM DAC
				 ***********************/
			case Port_DacPixelMask:
				RDAC.mask = data_v;
				return;

			case Port_DacIndexWrite:
				RDAC.index = data_v * 3u;
				RDAC.latch = 0x3u;
				return;

			case Port_DacIndexRead:
				RDAC.index = data_v * 3u;
				RDAC.latch = 0x0u;
				return;

			case Port_DacDataWrite:
				RDAC.color[RDAC.index] = data_v & 0x3Fu;
				RDAC.index += 1u;
				while (RDAC.index >= 0x300u)
					RDAC.index -= 0x300u;
				return;

				/***********************
				 *	CRT CONTROLLER
				 ***********************/
			case Port_MdaCrtIndex:
			case Port_VgaCrtIndex:
				if (port_v != port_crt_index) break;
				CRTC.index = data_v & 0x1Fu;
				return;

			case Port_MdaCrtData:
			case Port_VgaCrtData:
				if (port_v != port_crt_data) break;
				if (CRTC.index < std::size(CRTC.data) && (
            !(CRTC.data[0x11u] & 0x80u) 
            ||CRTC.index > 0x07u)) 
        {
  				CRTC.data[CRTC.index] = data_v;
				}
				return;

				/***********************
				 *	SEQUENCER
				 ***********************/
			case Port_SequencerIndex:
				SEQU.index = data_v & 0x7u;
				return;

			case Port_SequencerData:
        if (SEQU.index < std::size(SEQU.data))
				  SEQU.data[SEQU.index] = data_v;
				return;

				/*********************************
				 *	GRAHPICS CONTROLLER
				 *********************************/
			case Port_GraphicsCtrlIndex:
				GFXC.index = data_v & 0x0Fu;
				return;
			case Port_GraphicsCtrlData:
        if (GFXC.index < std::size(GFXC.data))
				  GFXC.data[GFXC.index] = data_v;
				return;

				/***********************
				 *	ATTRIBUTE CONTROLLER
				 ***********************/
			case Port_Attribute0:
				if (!attrib.latch) {
					attrib.latch = !attrib.latch;
					attrib.index_and_pas = data_v & 0x3Fu;
				} else {
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
				MISC.value = data_v;
				return ;

			case Port_MdaFeatureControl:
			case Port_VgaFeatureControl:
				if (port_v != port_control) break;
				feature_control = data_v;
				return ;

			default:
				break;
			}
			
			return ;
		}

		
		constexpr inline auto IoPortFetch(uint16_t port_v) noexcept -> uint8_t
		{
			uint8_t tmp_v{ 0 };
      auto const port_crt_data  = MISC.io_addr_3dx 
                                ? Port_VgaCrtData        
                                : Port_MdaCrtData ;
      auto const port_crt_index = MISC.io_addr_3dx 
                                ? Port_VgaCrtIndex       
                                : Port_MdaCrtIndex ;
      auto const port_status    = MISC.io_addr_3dx 
                                ? Port_VgaInputStatus
                                : Port_MdaInputStatus ;
      auto const port_control   = MISC.io_addr_3dx 
                                ? Port_VgaFeatureControl 
                                : Port_MdaFeatureControl ;
        
			switch (port_v)
			{
				/***********************
				 *	RAM DAC
				 *******************/
			case Port_DacPixelMask:
				return RDAC.mask;

			case Port_DacDataRead:
				tmp_v = RDAC.color[RDAC.index];
				RDAC.index += 1u;
				while (RDAC.index >= 0x300u)
					RDAC.index -= 0x300u;
				return tmp_v;

			case Port_DacStateRead:
				return RDAC.latch;

				/***********************
				 *	CRT CONTROLLER
				 ***********************/
			case Port_MdaCrtIndex:
			case Port_VgaCrtIndex:
				if (port_v != port_crt_index)
          break;
				return CRTC.index;        

			case Port_MdaCrtData:
			case Port_VgaCrtData:
				if (port_v != port_crt_data)
          break;
        tmp_v = 0xffu;
        if (CRTC.index < std::size(CRTC.data))
				  tmp_v = CRTC.data[CRTC.index];  
				return tmp_v;

			case Port_MdaInputStatus:
			case Port_VgaInputStatus:
				if (port_v != port_status)
          break;
				attrib.latch = false;
				return 0x00u;

				/***********************
				 *	SEQUENCER
				 ***********************/
			case Port_SequencerIndex:
				return SEQU.index;

			case Port_SequencerData:
        tmp_v = 0xffu;
        if (SEQU.index < std::size(SEQU.data))
				  tmp_v = SEQU.data[SEQU.index];
				return tmp_v;

				/**************************
				 *	GRAPHICS CONTROLLER
				 **************************/
			case Port_GraphicsCtrlIndex:
				return GFXC.index;

			case Port_GraphicsCtrlData:
        tmp_v = 0xffu;
        if (GFXC.index < std::size(GFXC.data))  
          tmp_v = GFXC.data[GFXC.index];
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
				return MISC.value;

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
			using enum ValueIndex;
			     if constexpr (_Index == EightDotMode              ) return SEQU.eight_dot_mode ;
			else if constexpr (_Index == DotClockRate              ) return SEQU.dot_clock_rate ;
      else if constexpr (_Index == ChainFourEnable           ) return SEQU.chain_four_enable ;
      else if constexpr (_Index == ReadMapSelect             ) return GFXC.read_map_select;
      else if constexpr (_Index == ReadModeSelect            ) return GFXC.read_mode;
      else if constexpr (_Index == WriteModeSelect           ) return GFXC.write_mode;
      else if constexpr (_Index == MasterClockSelect         ) return MISC.clock_select;

      else if constexpr (_Index == MaximumScanline           ) return CRTC.maximum_scan_line ;
			else if constexpr (_Index == HorizontalTotal           ) return CRTC.horizontal_total ;
			else if constexpr (_Index == HorizontalDisplayEnd      ) return CRTC.horizontal_display_end ;
			else if constexpr (_Index == HorizontalRetraceStart    ) return CRTC.horizontal_retrace_start ;      
			else if constexpr (_Index == HorizontalRetraceEnd      ) return CRTC.horizontal_retrace_end ;
			else if constexpr (_Index == HorizontalBlankingStart   ) return CRTC.horizontal_blanking_start ;     
			else if constexpr (_Index == HorizontalBlankingEnd     ) return CRTC.horizontal_blanking_end_0_4
						                                                        + CRTC.horizontal_blanking_end_5 ;			
			else if constexpr (_Index == VerticalTotal             ) return CRTC.vertical_total_0_7 * 0x1u
						                                                        + CRTC.vertical_total_8 * 0x100u 
						                                                        + CRTC.vertical_total_9 * 0x200u ;
			else if constexpr (_Index == VerticalDisplayEnd        ) return CRTC.vertical_display_end_0_7 * 0x1u
						                                                        + CRTC.vertical_display_end_8 * 0x100u 
					                                                          + CRTC.vertical_display_end_9 * 0x200u ;		
			else if constexpr (_Index == VerticalRetraceStart      ) return CRTC.vertical_retrace_start_0_7 * 0x1u
				 	                                                          + CRTC.vertical_retrace_start_8 * 0x100u 
				 	                                                          + CRTC.vertical_retrace_start_9 * 0x200u ;			
			else if constexpr (_Index == VerticalRetraceEnd        ) return CRTC.vertical_retrace_end ;
			else if constexpr (_Index == VerticalBlankingStart     ) return CRTC.vertical_blanking_start_0_7 * 0x1u
					                                                          + CRTC.vertical_blanking_start_8 * 0x100u 
					                                                          + CRTC.vertical_blanking_start_9 * 0x200u ;		
			else if constexpr (_Index == VerticalBlankingEnd       ) return CRTC.vertical_blanking_end ;	

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
					uint8_t :1;
					// 0x09
					uint8_t maximum_scan_line:5;
					uint8_t vertical_blanking_start_9:1;
					uint8_t line_compare_9:1;
					uint8_t scan_doubling:1;
					// 0x0A
					uint8_t cursor_line_start:5;
					uint8_t cursor_disable:1;
					uint8_t :2;

					// 0x0B
					uint8_t cursor_line_end:5;
					uint8_t cursor_skew:2;
					uint8_t :1;
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
					uint8_t :2;
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
					uint8_t :1;
					// 0x15
					uint8_t vertical_blanking_start_0_7:8;
					// 0x16
					uint8_t vertical_blanking_end:7;
					uint8_t :1;
					// 0x17
					uint8_t map_display_address_13:1;
					uint8_t map_display_address_14:1;
					uint8_t scanline_clock_divide_by_two:1;
					uint8_t memory_address_clock_divide_by_two:1;
					uint8_t :1;
					uint8_t address_wrap_select:1;
					uint8_t word_byte_mode_select:1;
					uint8_t sync_enable:1;					
					// 0x18
					uint8_t line_compare_0_7:8;
				};
			};
		} CRTC;

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
					uint8_t :6;
					// 0x01
					uint8_t eight_dot_mode:1;
					uint8_t :1;
					uint8_t shift_load_rate:1;
					uint8_t dot_clock_rate:1;
					uint8_t shift_four_enable:1;
					uint8_t screen_disable:1;
					uint8_t :2;
					// 0x02
					uint8_t memory_plane_write_mask:4;
					uint8_t :4;
					// 0x03
					uint8_t character_map_select_b_0_1:2;
					uint8_t character_map_select_a_0_1:2;
					uint8_t character_map_select_b_2:1;
					uint8_t character_map_select_a_2:1;
					uint8_t :2;
					// 0x04
					uint8_t :1;
					uint8_t extended_memory_enable:1;
					uint8_t host_odd_even_write_addressing_disable:1;
					uint8_t chain_four_enable:1;
					uint8_t :4;
				};
			};
		} SEQU;

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
					uint8_t :4;
					// 0x01
					uint8_t enable_set_or_reset:4;
					uint8_t :4;
					// 0x02
					uint8_t color_compare:4;
					uint8_t :4;
					// 0x03
					uint8_t rotate_count:3;
					uint8_t logical_operation:2;
					uint8_t :3;
					// 0x04
					uint8_t read_map_select:2;
					uint8_t :6;
					// 0x05
					uint8_t write_mode:2;
					uint8_t :1;
					uint8_t read_mode:1;
					uint8_t host_odd_even_read_addressing_enable:1;
					uint8_t shift_register_interlieve_mode:1;
					uint8_t shift_256_color_mode:1;
					uint8_t :1;
					// 0x06
					uint8_t alphanumeric_mode_disable:1;
					uint8_t chain_odd_even_enable:1;
					uint8_t memory_map_select:2;
					uint8_t :4;
					// 0x07
					uint8_t color_dont_care:4;
					uint8_t :4;
					// 0x08
					uint8_t bit_mask;
				};
			};

		} GFXC;

		struct
		{
			uint16_t index:16;
			uint8_t mask:8;
			uint8_t latch:8;
			uint8_t flags:8;
			uint8_t color[256u * 3u];
		} RDAC;

		struct
		{
			bool latch;

			union
			{
				struct {
					uint8_t index:5;
					uint8_t pas:1;
					uint8_t :2;
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
					uint8_t :1;
					uint8_t pixel_panning_mode:1;
					uint8_t eight_bit_color_enable:1;
					uint8_t palette_bits_5_4_select:1;
					// 0x11
					uint8_t overscan_color;
					// 0x12
					uint8_t color_plane_enable:4;
					uint8_t :4;
					// 0x13
					uint8_t horizontal_panning:4;
					uint8_t :4;
					// 0x14
					uint8_t color_select_5_4:2;
					uint8_t color_select_7_6:2;
					uint8_t :4;
				};
			};
		} attrib;
		union {
			uint8_t value:8;
			struct {				
				uint8_t io_addr_3dx:1;
				uint8_t ram_access_enable:1;
				uint8_t clock_select:2;
				uint8_t :1;
				uint8_t odd_even_page_select:1;
				uint8_t sync_polarity:2;
			};
		} MISC;
		uint8_t feature_control:8;

	};
#pragma pack(pop)	
  static_assert(sizeof(VGARegisters) == 0x400u);
}
