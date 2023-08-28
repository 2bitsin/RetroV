#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include <win32/winhvpx.hpp>

namespace core
{
	enum Access: uint32_t {
		kAccessNone = 0x0u,
		kAccessFetch = 0x1u,
		kAccessWrite = 0x2u,
		kAccessExecute = 0x4u,
		kTrackDirty = 0x8u
	};

	DEFINE_ENUM_FLAG_OPERATORS(Access)

	static inline const auto kAccessMemory = kAccessFetch | kAccessWrite | kAccessExecute;
	static inline const auto kAccessReadOnly = kAccessFetch | kAccessExecute;
	static inline const auto kAccessDevice = kAccessFetch | kAccessWrite;

	static inline auto to_string(core::Access access_v) -> std::string
	{
		using enum core::Access;
		std::string result_v;
		if (access_v & kAccessFetch   ) result_v += "R";
		if (access_v & kAccessWrite   ) result_v += "W";
		if (access_v & kAccessExecute ) result_v += "X";
		if (access_v & kTrackDirty    ) result_v += "D";
		return result_v;
	}
}