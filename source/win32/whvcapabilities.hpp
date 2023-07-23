#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <type_traits>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

namespace win32 {

	struct WHvCapabilities
	{

		static auto IsVendorAMD() -> bool;
		static auto IsVendorIntel() -> bool;
		static auto Get(WHV_CAPABILITY_CODE, void* buffer_v, std::uint32_t length_v) -> std::uint32_t;

		template <typename T> requires (std::is_trivial_v<T>)
		static inline auto Get(WHV_CAPABILITY_CODE code_v) -> T {
			T buffer_v{ };
			auto const length_v = Get(code_v, &buffer_v, sizeof(buffer_v));
			if (length_v != sizeof(buffer_v))
				throw std::invalid_argument("Invalid buffer size");
			return buffer_v;
		}

		static auto LogInformation() -> void;

	};
}
