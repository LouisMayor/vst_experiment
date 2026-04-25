#pragma once

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/vsttypes.h"
#include <cmath>

namespace VstCommon {

struct VolumeModel {
	static constexpr Steinberg::Vst::ParamID	  kId			= 101;
	static constexpr const Steinberg::Vst::TChar *kTitle		= STR16("Volume");
	static constexpr const Steinberg::Vst::TChar *kUnits		= STR16("dB");
	static constexpr Steinberg::Vst::ParamValue	  kMinPlain		= -60.0;
	static constexpr Steinberg::Vst::ParamValue	  kMaxPlain		= 0.0;
	static constexpr Steinberg::Vst::ParamValue	  kDefaultPlain = -18.0;
	static constexpr Steinberg::int32			  kFlags		= Steinberg::Vst::ParameterInfo::kCanAutomate;

	static Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue norm) {
		return kMinPlain + norm * (kMaxPlain - kMinPlain);
	}

	static Steinberg::Vst::ParamValue toGain(Steinberg::Vst::ParamValue norm) {
		auto db = toPlain(norm);
		return std::pow(10.0, db / 20.0);
	}

	static Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue plain) {
		return (plain - kMinPlain) / (kMaxPlain - kMinPlain);
	}

	static void toString(Steinberg::Vst::ParamValue norm, Steinberg::Vst::String128 string) {
		auto			   db = toPlain(norm);
		Steinberg::UString wrapper(string, USTRINGSIZE(Steinberg::Vst::String128));
		wrapper.printFloat(db, 1);
	}

	static bool fromString(const Steinberg::Vst::TChar *str, Steinberg::Vst::ParamValue &norm) {
		Steinberg::UString ustr(const_cast<Steinberg::Vst::TChar *>(str), USTRINGSIZE(Steinberg::Vst::String128));
		double			   db = 0.0;
		if (!ustr.scanFloat(db))
			return false;
		norm = toNormalized(db);
		return true;
	}
};

} // namespace VstCommon
