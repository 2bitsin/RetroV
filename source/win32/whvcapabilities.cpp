#include <win32/whvcapabilities.hpp>
#include <utils/logger.hpp>

using win32::WHvCapabilities;

auto WHvCapabilities::Get(WHV_CAPABILITY_CODE code_v, void* buffer_v, std::uint32_t length_v) -> std::uint32_t
{
	WIN32_ERROR_ASSERT(WHvGetCapability(code_v, buffer_v, length_v, &length_v));
	return length_v;
}

auto WHvCapabilities::LogInformation() -> void
{
	using namespace std::string_view_literals;
	using namespace std::string_literals;
	using utils::logger;

	auto hypervisor_present_v = Get<BOOL>(WHvCapabilityCodeHypervisorPresent);
	auto processor_vendor_v = Get<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	auto processor_features_v = Get<WHV_PROCESSOR_FEATURES>(WHvCapabilityCodeProcessorFeatures);
	auto synthetic_features_v = Get<WHV_SYNTHETIC_PROCESSOR_FEATURES_BANKS>(WHvCapabilityCodeSyntheticProcessorFeaturesBanks);


#define Q(X, Y) std::format("\n  * {:.<55} {}", #Y, (X.Y ? ": Yes" : ": No"))

	logger::info(logger::deflog, 
		"\n  Hypervisor present ........ : {}."
		"\n  Processor vendor .......... : {}."		
		"\n  Processor features ........ : {} "		
		"\n"

		, hypervisor_present_v ? "Yes" : "No"
		, ([] (auto vendor_v) {
			  switch (vendor_v) {
				case WHvProcessorVendorAmd:		return "AMD"sv;
				case WHvProcessorVendorIntel: return "Intel"sv;
				case WHvProcessorVendorHygon: return "Hygan"sv;
				default: return "Unknown"sv;
			}}) (processor_vendor_v)
			, ""s
			+ Q(processor_features_v, Sse3Support)
			+ Q(processor_features_v, LahfSahfSupport)
			+ Q(processor_features_v, Ssse3Support)
			+ Q(processor_features_v, Sse4_1Support)
			+ Q(processor_features_v, Sse4_2Support)
			+ Q(processor_features_v, Sse4aSupport)
			+ Q(processor_features_v, XopSupport)
			+ Q(processor_features_v, PopCntSupport)
			+ Q(processor_features_v, Cmpxchg16bSupport)
			+ Q(processor_features_v, Altmovcr8Support)
			+ Q(processor_features_v, LzcntSupport)
			+ Q(processor_features_v, MisAlignSseSupport)
			+ Q(processor_features_v, MmxExtSupport)
			+ Q(processor_features_v, Amd3DNowSupport)
			+ Q(processor_features_v, ExtendedAmd3DNowSupport)
			+ Q(processor_features_v, Page1GbSupport)
			+ Q(processor_features_v, AesSupport)
			+ Q(processor_features_v, PclmulqdqSupport)
			+ Q(processor_features_v, PcidSupport)
			+ Q(processor_features_v, Fma4Support)
			+ Q(processor_features_v, F16CSupport)
			+ Q(processor_features_v, RdRandSupport)
			+ Q(processor_features_v, RdWrFsGsSupport)
			+ Q(processor_features_v, SmepSupport)
			+ Q(processor_features_v, EnhancedFastStringSupport)
			+ Q(processor_features_v, Bmi1Support)
			+ Q(processor_features_v, Bmi2Support)
			+ Q(processor_features_v, MovbeSupport)
			+ Q(processor_features_v, Npiep1Support)
			+ Q(processor_features_v, DepX87FPUSaveSupport)
			+ Q(processor_features_v, RdSeedSupport)
			+ Q(processor_features_v, AdxSupport)
			+ Q(processor_features_v, IntelPrefetchSupport)
			+ Q(processor_features_v, SmapSupport)
			+ Q(processor_features_v, HleSupport)
			+ Q(processor_features_v, RtmSupport)
			+ Q(processor_features_v, RdtscpSupport)
			+ Q(processor_features_v, ClflushoptSupport)
			+ Q(processor_features_v, ClwbSupport)
			+ Q(processor_features_v, ShaSupport)
			+ Q(processor_features_v, X87PointersSavedSupport)
			+ Q(processor_features_v, InvpcidSupport)
			+ Q(processor_features_v, IbrsSupport)
			+ Q(processor_features_v, StibpSupport)
			+ Q(processor_features_v, IbpbSupport)
			+ Q(processor_features_v, SsbdSupport)
			+ Q(processor_features_v, FastShortRepMovSupport)
			+ Q(processor_features_v, RdclNo)
			+ Q(processor_features_v, IbrsAllSupport)
			+ Q(processor_features_v, SsbNo)
			+ Q(processor_features_v, RsbANo)
			+ Q(processor_features_v, RdPidSupport)
			+ Q(processor_features_v, UmipSupport)
			+ Q(processor_features_v, MdsNoSupport)
			+ Q(processor_features_v, MdClearSupport)
			+ Q(processor_features_v, TaaNoSupport)
			+ Q(processor_features_v, TsxCtrlSupport)

			+ Q(synthetic_features_v, Bank0.HypervisorPresent)
			+ Q(synthetic_features_v, Bank0.Hv1)
			+ Q(synthetic_features_v, Bank0.AccessVpRunTimeReg)
			+ Q(synthetic_features_v, Bank0.AccessPartitionReferenceCounter)
			+ Q(synthetic_features_v, Bank0.AccessSynicRegs)
			+ Q(synthetic_features_v, Bank0.AccessSyntheticTimerRegs)
			+ Q(synthetic_features_v, Bank0.AccessIntrCtrlRegs)
			+ Q(synthetic_features_v, Bank0.AccessHypercallRegs)
			+ Q(synthetic_features_v, Bank0.AccessVpIndex)
			+ Q(synthetic_features_v, Bank0.AccessPartitionReferenceTsc)
			+ Q(synthetic_features_v, Bank0.AccessGuestIdleReg)
			+ Q(synthetic_features_v, Bank0.AccessFrequencyRegs)
			+ Q(synthetic_features_v, Bank0.EnableExtendedGvaRangesForFlushVirtualAddressList)
			+ Q(synthetic_features_v, Bank0.FastHypercallOutput)
			+ Q(synthetic_features_v, Bank0.DirectSyntheticTimers)
			+ Q(synthetic_features_v, Bank0.ExtendedProcessorMasks)
			+ Q(synthetic_features_v, Bank0.TbFlushHypercalls)
			+ Q(synthetic_features_v, Bank0.SyntheticClusterIpi)
			+ Q(synthetic_features_v, Bank0.NotifyLongSpinWait)
			+ Q(synthetic_features_v, Bank0.QueryNumaDistance)
			+ Q(synthetic_features_v, Bank0.SignalEvents)
			+ Q(synthetic_features_v, Bank0.RetargetDeviceInterrupt)	
			);

#undef Q
}

auto WHvCapabilities::IsVendorIntel() -> bool {
	auto const vendor_v = Get<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorIntel;
}

auto WHvCapabilities::IsVendorAMD() -> bool {
	auto const vendor_v = Get<WHV_PROCESSOR_VENDOR>(WHvCapabilityCodeProcessorVendor);
	return vendor_v == WHvProcessorVendorAmd || vendor_v == WHvProcessorVendorHygon;
}
