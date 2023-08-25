#pragma once

#include <cstdint>
#include <cstddef>

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

}