#pragma once

#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "vst_common/parameters.h"

namespace GainVendor {
constexpr VstCommon::parameter_definition gain_param{
	.id						  = 1,
	.title					  = STR16("Gain"),
	.units					  = STR16("%"),
	.step_count				  = 0,
	.default_normalized_value = 1.,
	.flags					  = Steinberg::Vst::ParameterInfo::kCanAutomate,
};
}
