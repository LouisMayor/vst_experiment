#pragma once

#include "public.sdk/source/vst/utility/sampleaccurate.h"

namespace VstCommon {

/**
 * DSP adapter: wraps Steinberg::Vst::SampleAccurate::Parameter for a given ParameterModel.
 *
 * Provides compile-time access to the Model's conversion logic via getPlainValue().
 *
 * Usage:
 *   VstCommon::ModelSampleAccurateParameter<MyModel> param;
 *   param.beginChanges(queue);
 *   auto plain = param.advancePlain(numSamples);  // advance + convert
 *   // or
 *   param.advance(numSamples);
 *   auto plain = param.getPlainValue();           // convert current value
 */
template <typename Model> class ModelSampleAccurateParameter : public Steinberg::Vst::SampleAccurate::Parameter {
  public:
	ModelSampleAccurateParameter() : Steinberg::Vst::SampleAccurate::Parameter(Model::kId, Model::toNormalized(Model::kDefaultPlain)) {}

	/**
	 * Convert the current normalized value to a plain value using Model::toPlain.
	 */
	Steinberg::Vst::ParamValue getPlainValue() const {
		return Model::toPlain(getValue());
	}

	/**
	 * Advance the parameter by numSamples and return the new plain value.
	 */
	Steinberg::Vst::ParamValue advancePlain(Steinberg::int32 numSamples) {
		advance(numSamples);
		return getPlainValue();
	}
};

} // namespace VstCommon
