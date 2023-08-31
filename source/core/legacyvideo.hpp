#pragma once

#include <cstdint>
#include <cstddef>

#include <deque>

#include <list>

#include <core/display.hpp>
#include <core/memory.hpp>

#include <utils/smart_span.hpp>
#include <utils/span.hpp>

namespace core
{
	struct Machine;
	struct Processor;

	struct LegacyVideo
	{
		LegacyVideo(Machine& machine_v);

		auto Initialize() -> void;

		auto Start() -> void;
		auto Stop() -> void;
		auto Restart() -> void;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;

		auto Refresh(Display::surface_tmp& surface_v) -> void;

	private:
		Machine& m_Machine;
		std::uint16_t m_Height;
		std::uint16_t m_Width;

		std::optional<Memory> m_CharacterWindow;
		std::optional<Memory> m_GraphicalWindow;
		win32::unique_span<std::byte> m_TemporaryBuffer;
	};
}