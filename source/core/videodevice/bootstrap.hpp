#pragma once

#include <core/videodevice/interface.hpp>

namespace core::videodevice
{

	struct Bootstrap : Interface
	{
		using Interface::Interface;
		using Interface::duration_type;

		auto Refresh(Display&) -> duration_type override;
		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t override;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t override;

		~Bootstrap() override = default;
	};
}
