#pragma once

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>
#include <win32/error.hpp>

#include <utils/span.hpp>
#include <core/hypervisor_fwd.hpp>

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace core
{

	struct Partition
	{
		Partition(Hypervisor& hypervisor_v);

		auto operator = (Partition const&) -> Partition & = delete;
		Partition(Partition const&) = delete;

		auto operator = (Partition&&) noexcept -> Partition&;
		Partition(Partition&&) noexcept;

		auto Swap(Partition& other_v) noexcept -> void;

	  ~Partition();

		auto GetHandle() const -> WHV_PARTITION_HANDLE;
		
		auto Setup() const -> void;

		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte const> buffer_v) const -> void;
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v, std::span<std::byte>& buffer_v) const -> void;

		template <typename T> requires (std::is_trivial_v<T>)
		auto SetProperty(WHV_PARTITION_PROPERTY_CODE code_v, T const& value_v) const -> void
		{
			return SetProperty(code_v, utils::as_bytes(value_v));
		}

		template <typename T> requires (std::is_trivial_v<T>)
		auto GetProperty(WHV_PARTITION_PROPERTY_CODE code_v) const -> T
		{
			T result_v{ };
			auto buffer_v = utils::as_mutable_bytes(result_v);
			GetProperty(code_v, buffer_v);
			if (buffer_v.size() != sizeof(T)) {
				throw std::invalid_argument{ "Wrong argument size" };
			}
			return result_v;
		}

	private:
		core::Hypervisor* m_Hypervisor;
		WHV_PARTITION_HANDLE m_Handle;
	};

}
