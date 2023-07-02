#pragma once

#include <core/hypervisor_fwd.hpp>

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
		
		virtual auto Emulate(std::stop_token const& token_v) -> void = 0;		
		virtual auto GetCategory() const noexcept -> DeviceCatory;
	};

	using InterfacePtr = std::unique_ptr<Interface>;
	using Config = std::any;

	static auto CreateDevice(core::Hypervisor&, std::string_view device_name_v, Config const&) -> InterfacePtr;
}