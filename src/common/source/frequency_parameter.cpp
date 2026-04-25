#include "vst_common/frequency_parameter.h"

#include "pluginterfaces/base/ustring.h"

#include <cmath>
#include <cstdio>

namespace VstCommon {

static Steinberg::Vst::ParamValue freq_to_k(Steinberg::Vst::ParamValue hz, Steinberg::Vst::ParamValue minHz, Steinberg::Vst::ParamValue maxHz) {
	auto minK = std::log10(minHz);
	auto maxK = std::log10(maxHz);
	return (std::log10(hz) - minK) / (maxK - minK);
}

static Steinberg::Vst::ParamValue k_to_freq(Steinberg::Vst::ParamValue normalized, Steinberg::Vst::ParamValue minHz, Steinberg::Vst::ParamValue maxHz) {
	auto minK = std::log10(minHz);
	auto maxK = std::log10(maxHz);
	return std::pow(10., minK + normalized * (maxK - minK));
}

FrequencyParameter::FrequencyParameter(const Steinberg::Vst::TChar *title, Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue minHz,
									   Steinberg::Vst::ParamValue maxHz, Steinberg::Vst::ParamValue defaultHz, Steinberg::int32 flags,
									   Steinberg::Vst::UnitID unitID)
	: RangeParameter(title, tag, STR16("Hz"), minHz, maxHz, defaultHz, 0, flags, unitID) {
	auto defaultNorm			= freq_to_k(defaultHz, minHz, maxHz);
	info.defaultNormalizedValue = defaultNorm;
	setNormalized(defaultNorm);
}

void FrequencyParameter::toString(Steinberg::Vst::ParamValue valueNormalized, Steinberg::Vst::String128 string) const {
	auto			   hz = k_to_freq(valueNormalized, minPlain, maxPlain);
	Steinberg::UString wrapper(string, USTRINGSIZE(Steinberg::Vst::String128));
	if (hz >= 1000.)
		wrapper.printFloat(hz / 1000., 2);
	else
		wrapper.printFloat(hz, 2);
}

bool FrequencyParameter::fromString(const Steinberg::Vst::TChar *string, Steinberg::Vst::ParamValue &valueNormalized) const {
	Steinberg::UString ustr(const_cast<Steinberg::Vst::TChar *>(string), USTRINGSIZE(Steinberg::Vst::String128));
	double			   hz = 0.;
	if (!ustr.scanFloat(hz))
		return false;
	valueNormalized = freq_to_k(hz, minPlain, maxPlain);
	return true;
}

} // namespace VstCommon
