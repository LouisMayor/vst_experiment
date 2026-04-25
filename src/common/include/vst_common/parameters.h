#pragma once

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

namespace VstCommon {

struct parameter_definition {
	Steinberg::Vst::ParamID id;
	const Steinberg::Vst::TChar *title;
	const Steinberg::Vst::TChar *units;
	Steinberg::int32 step_count;
	Steinberg::Vst::ParamValue default_normalized_value;
	Steinberg::int32 flags;
};

}
