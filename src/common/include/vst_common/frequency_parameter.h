#pragma once

#include "public.sdk/source/vst/vstparameters.h"

namespace VstCommon {

class FrequencyParameter : public Steinberg::Vst::RangeParameter {
  public:
	FrequencyParameter(const Steinberg::Vst::TChar *title, Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue minHz = 20.,
					   Steinberg::Vst::ParamValue maxHz = 20000., Steinberg::Vst::ParamValue defaultHz = 1000.,
					   Steinberg::int32 flags = Steinberg::Vst::ParameterInfo::kCanAutomate, Steinberg::Vst::UnitID unitID = Steinberg::Vst::kRootUnitId);

	void toString(Steinberg::Vst::ParamValue valueNormalized, Steinberg::Vst::String128 string) const SMTG_OVERRIDE;
	bool fromString(const Steinberg::Vst::TChar *string, Steinberg::Vst::ParamValue &valueNormalized) const SMTG_OVERRIDE;

	OBJ_METHODS(FrequencyParameter, RangeParameter)
};

} // namespace VstCommon
