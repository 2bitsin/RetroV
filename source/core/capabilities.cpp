#include <core/capabilities.hpp>

using core::Capabilities;

auto Capabilities::Get(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto Capabilities::IsVendorIntel() -> bool {
	auto const vendor_v = Get<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorIntel;
}

auto Capabilities::IsVendorAMD() -> bool {
	auto const vendor_v = Get<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorAmd || vendor_v == WHvProcessorVendorHygon;
}
