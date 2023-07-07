#pragma once

namespace core::processor
{
	static inline constexpr const auto kCarryFlag                   = 0x000001u;
	static inline constexpr const auto kParityFlag                  = 0x000004u;
	static inline constexpr const auto kAdjustFlag                  = 0x000010u;
	static inline constexpr const auto kZeroFlag                    = 0x000040u;
	static inline constexpr const auto kSignFlag                    = 0x000080u;
	static inline constexpr const auto kTrapFlag                    = 0x000100u;
	static inline constexpr const auto kInterruptFlag               = 0x000200u;
	static inline constexpr const auto kDirectionFlag               = 0x000400u;
	static inline constexpr const auto kOverflowFlag                = 0x000800u;
	static inline constexpr const auto kPrivilegeLevelMask          = 0x003000u;
	static inline constexpr const auto kNestedTaskFlag              = 0x004000u;
	static inline constexpr const auto kResumeFlag                  = 0x010000u;
	static inline constexpr const auto kVirtual8086ModeFlag         = 0x020000u;
	static inline constexpr const auto kAlignmentCheckFlag          = 0x040000u;
	static inline constexpr const auto kVirtualInterruptFlag        = 0x080000u;
	static inline constexpr const auto kVirtualInterruptPendingFlag = 0x100000u;
	static inline constexpr const auto kCpuIdAvailableFlag          = 0x200000u;
}