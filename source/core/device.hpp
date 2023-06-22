#pragma once

#include <cstddef>
#include <cstdint>

namespace core
{
	struct Machine;

	struct Device 
	{
		void Setup (Machine& machine_v);
	};
}