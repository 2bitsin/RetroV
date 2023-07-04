#pragma once

#include <core/hypervisor_fwd.hpp>
#include <core/cpu/processor_fwd.hpp>
#include <core/scheduler_fwd.hpp>
#include <core/service_fwd.hpp>

#include <string_view>
#include <stop_token>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <any>

namespace device
{
	enum class DeviceCatory: 
		std::uint32_t
	{
		kGeneric,
		kDebug,
		kVideo,
		kAudio,
		kInput,
		kStorage,
		kNetwork
	};

	struct Interface
	{
		Interface() = default;
		virtual ~Interface() = default;
		
		Interface(Interface const&) = delete;
		Interface(Interface&&) = delete;
		auto operator=(Interface const&) -> Interface& = delete;
		auto operator=(Interface&&) -> Interface& = delete;
		
		virtual auto GetCategory() const noexcept -> DeviceCatory;
		virtual auto Emulate(core::Scheduler&, core::Service&) -> void = 0;		
		virtual auto Resume() -> void = 0;
		virtual auto Pause() -> void = 0;

	};

	using InterfacePtr = std::unique_ptr<Interface>;
	using Config = std::any;

	auto CreateDevice(core::Hypervisor&, std::string_view device_name_v, Config const&) -> InterfacePtr;
}