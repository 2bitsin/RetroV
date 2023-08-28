#pragma once

#include <cstdint>
#include <cstddef>

#include <deque>

#include <list>

#include <utils/smart_span.hpp>
#include <utils/span.hpp>
#include <utils/buffer2d.hpp>

#include <core/memory.hpp>

namespace core
{
	struct Machine;
	struct Processor;

	struct LegacyVideo
	{
		using raw_buffer_type = utils::buffer2d<std::uint32_t>;

		struct buffer_deleter
		{ buffer_deleter(LegacyVideo& legacyvideo_v) : m_legacyvideo(legacyvideo_v) {}
			auto operator()(raw_buffer_type* buffer_v) const -> void { m_legacyvideo.ReleaseBuffer(buffer_v); }
			LegacyVideo& m_legacyvideo; };
	
		using buffer_type = std::unique_ptr<raw_buffer_type, buffer_deleter>;

		LegacyVideo(Machine& machine_v);

		auto Initialize() -> std::int32_t;

		auto IoPortAccess(Processor const& vcpu_v, bool is_write_v, std::uint16_t port_v, utils::limited_span<std::byte, 4u> data_v) -> std::int32_t;
		auto MemoryAccess(Processor const& vcpu_v, bool is_write_v, std::uint64_t addr_v, utils::limited_span<std::byte, 8u> data_v) -> std::int32_t;

		auto Render() -> std::tuple<buffer_type, std::chrono::microseconds>;

	protected:

		friend struct buffer_deleter;

		auto ReleaseBuffer(raw_buffer_type* buffer_ptr) -> void;
		auto AcquireBuffer() -> buffer_type;

	private:
		Machine& m_Machine;
		std::uint16_t m_Height;
		std::uint16_t m_Width;

		std::optional<Memory> m_CharacterWindow;
		std::optional<Memory> m_GraphicalWindow;
		win32::unique_span<std::byte> m_TemporaryBuffer;

		std::deque<std::unique_ptr<raw_buffer_type>> m_BufferPool;
	};
}