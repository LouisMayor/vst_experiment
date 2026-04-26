#pragma once

#include "public.sdk/source/vst/vstparameters.h"
#include <type_traits>

namespace VstCommon {

namespace detail {

template <typename Model, typename = void> struct model_step_count {
	static constexpr Steinberg::int32 value = 0;
};

template <typename Model> struct model_step_count<Model, std::void_t<decltype(Model::kStepCount)>> {
	static constexpr Steinberg::int32 value = static_cast<Steinberg::int32>(Model::kStepCount);
};

} // namespace detail

/**
 * Controller adapter: wraps Steinberg::Vst::RangeParameter for a given ParameterModel.
 *
 * All virtual overrides (toPlain, toNormalized, toString, fromString) are
 * dispatched to the Model's static methods at compile time — zero runtime overhead.
 */
template <typename Model> class ModelRangeParameter : public Steinberg::Vst::RangeParameter {
  public:
	ModelRangeParameter()
		: RangeParameter(Model::kTitle, Model::kId, Model::kUnits, Model::kMinPlain, Model::kMaxPlain, Model::kDefaultPlain,
						 detail::model_step_count<Model>::value, Model::kFlags, Steinberg::Vst::kRootUnitId) {
		auto defaultNorm			= Model::toNormalized(Model::kDefaultPlain);
		info.defaultNormalizedValue = defaultNorm;
		setNormalized(defaultNorm);
	}

	Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue norm) const SMTG_OVERRIDE {
		return Model::toPlain(norm);
	}

	Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue plain) const SMTG_OVERRIDE {
		return Model::toNormalized(plain);
	}

	void toString(Steinberg::Vst::ParamValue norm, Steinberg::Vst::String128 string) const SMTG_OVERRIDE {
		Model::toString(norm, string);
	}

	bool fromString(const Steinberg::Vst::TChar *str, Steinberg::Vst::ParamValue &norm) const SMTG_OVERRIDE {
		return Model::fromString(str, norm);
	}

	OBJ_METHODS(ModelRangeParameter, RangeParameter)
};

} // namespace VstCommon
