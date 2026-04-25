#pragma once

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/vsttypes.h"
#include <cmath>
#include <cstdio>

namespace VstCommon {

/**
 * Example ParameterModel for a logarithmic frequency parameter.
 *
 * Plain values are in Hz. The normalized<->plain mapping is logarithmic
 * to provide equal perceived resolution across the range.
 */
struct FrequencyModel {
	static constexpr Steinberg::Vst::ParamID	  kId			= 100;
	static constexpr const Steinberg::Vst::TChar *kTitle		= STR16("Frequency");
	static constexpr const Steinberg::Vst::TChar *kUnits		= STR16("Hz");
	static constexpr Steinberg::Vst::ParamValue	  kMinPlain		= 20.0;
	static constexpr Steinberg::Vst::ParamValue	  kMaxPlain		= 20000.0;
	static constexpr Steinberg::Vst::ParamValue	  kDefaultPlain = 1000.0;
	static constexpr Steinberg::int32			  kFlags		= Steinberg::Vst::ParameterInfo::kCanAutomate;

	static Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue norm) {
		auto minK = std::log10(kMinPlain);
		auto maxK = std::log10(kMaxPlain);
		return std::pow(10.0, minK + norm * (maxK - minK));
	}

	static Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue plain) {
		auto minK = std::log10(kMinPlain);
		auto maxK = std::log10(kMaxPlain);
		return (std::log10(plain) - minK) / (maxK - minK);
	}

	static void toString(Steinberg::Vst::ParamValue norm, Steinberg::Vst::String128 string) {
		auto hz = toPlain(norm);

		char text[32];
		if (hz >= 1000.0)
			snprintf(text, sizeof(text), "%.2f kHz", hz / 1000.0);
		else
			snprintf(text, sizeof(text), "%.0f Hz", hz);

		Steinberg::UString fmt_str = USTRING(text);
		Steinberg::UString(string, 128).append(fmt_str);
	}

	static bool fromString(const Steinberg::Vst::TChar *str, Steinberg::Vst::ParamValue &norm) {
		Steinberg::UString ustr(const_cast<Steinberg::Vst::TChar *>(str), USTRINGSIZE(Steinberg::Vst::String128));
		double			   hz = 0.0;
		if (!ustr.scanFloat(hz))
			return false;
		norm = toNormalized(hz);
		return true;
	}
};

} // namespace VstCommon
