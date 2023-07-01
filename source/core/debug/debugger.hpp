#pragma once

#include <win32/error.hpp>
#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/capstone.hpp>
#include <core/hypervisor_fwd.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core::debug
{
	struct Debugger
	{
		Debugger (Hypervisor& hypervisor_v);


	};
} 