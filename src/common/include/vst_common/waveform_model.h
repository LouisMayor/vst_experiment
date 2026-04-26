#pragma once

#include "generic_list_model.h"
#include <array>
#include <cstddef>

namespace VstCommon {

struct WaveformDefinition {
	static constexpr Steinberg::Vst::ParamID	  kId	 = 102;
	static constexpr const Steinberg::Vst::TChar *kTitle = STR16("Waveform");
	static constexpr const Steinberg::Vst::TChar *kUnits = STR16("");
	static constexpr Steinberg::int32			  kFlags = Steinberg::Vst::ParameterInfo::kCanAutomate;

	static constexpr std::array<Steinberg::Vst::ParamValue, 3>	  kValues		= {0.0, 1.0, 2.0};
	static constexpr std::array<const Steinberg::Vst::TChar *, 3> kLabels		= {STR16("Sine"), STR16("Saw"), STR16("Square")};
	static constexpr std::size_t								  kDefaultIndex = 0;
};

using WaveformModel = GenericListModel<WaveformDefinition>;

} // namespace VstCommon
