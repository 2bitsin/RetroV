#pragma once

#include <cstdint>
#include <cstddef>

namespace core::videodevice
{
  static inline constexpr const auto Port_MdaCrtIndex        = (uint16_t)0x3B4u;
  static inline constexpr const auto Port_MdaCrtData         = (uint16_t)0x3B5u;
  static inline constexpr const auto Port_MdaInputStatus     = (uint16_t)0x3BAu;
  static inline constexpr const auto Port_MdaFeatureControl  = (uint16_t)0x3BAu;
  static inline constexpr const auto Port_Attribute0         = (uint16_t)0x3C0u;
  static inline constexpr const auto Port_Attribute1         = (uint16_t)0x3C1u;
  static inline constexpr const auto Port_InputStatus        = (uint16_t)0x3C2u;
  static inline constexpr const auto Port_MiscOutputWrite    = (uint16_t)0x3C2u;
  static inline constexpr const auto Port_SequencerIndex     = (uint16_t)0x3C4u;
  static inline constexpr const auto Port_SequencerData      = (uint16_t)0x3C5u;
  static inline constexpr const auto Port_DacPixelMask       = (uint16_t)0x3C6u;
  static inline constexpr const auto Port_DacStateRead       = (uint16_t)0x3C7u;
  static inline constexpr const auto Port_DacIndexRead       = (uint16_t)0x3C7u;
  static inline constexpr const auto Port_DacIndexWrite      = (uint16_t)0x3C8u;
  static inline constexpr const auto Port_DacDataRead        = (uint16_t)0x3C9u;
  static inline constexpr const auto Port_DacDataWrite       = (uint16_t)0x3C9u;
  static inline constexpr const auto Port_FeatureControlRead = (uint16_t)0x3CAu;
  static inline constexpr const auto Port_MiscOutputRead     = (uint16_t)0x3CCu;
  static inline constexpr const auto Port_GraphicsCtrlIndex  = (uint16_t)0x3CEu;
  static inline constexpr const auto Port_GraphicsCtrlData   = (uint16_t)0x3CFu;
  static inline constexpr const auto Port_VgaCrtIndex        = (uint16_t)0x3D4u;
  static inline constexpr const auto Port_VgaCrtData         = (uint16_t)0x3D5u;
  static inline constexpr const auto Port_VgaInputStatus     = (uint16_t)0x3DAu;
  static inline constexpr const auto Port_VgaFeatureControl  = (uint16_t)0x3DAu;

  static inline constexpr const auto SyncPolarity_350 = 0b10u;
  static inline constexpr const auto SyncPolarity_400 = 0b01u;
  static inline constexpr const auto SyncPolarity_480 = 0b11u;

  static inline constexpr const auto ClockSelect_25MHz = 0b00u;
  static inline constexpr const auto ClockSelect_28MHz = 0b01u;
  // Custom
  static inline constexpr const auto ClockSelect_31MHz = 0b10u;
  static inline constexpr const auto ClockSelect_40MHz = 0b11u;

}