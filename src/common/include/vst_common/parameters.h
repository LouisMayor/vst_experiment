#pragma once

#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

namespace VstCommon {

/**
 * Simple POD definition for basic parameters.
 *
 * For parameters requiring custom conversion logic (logarithmic scaling,
 * specialized string formatting, etc.), prefer defining a ParameterModel
 * and using ModelRangeParameter / ModelSampleAccurateParameter.
 *
 * @see parameter_model.h
 */
struct parameter_definition {
	Steinberg::Vst::ParamID		 id;
	const Steinberg::Vst::TChar *title;
	const Steinberg::Vst::TChar *units;
	Steinberg::int32			 step_count;
	Steinberg::Vst::ParamValue	 default_normalized_value;
	Steinberg::int32			 flags;
};

} // namespace VstCommon
