#pragma once

#include "generic_list_parameter.h"
#include "waveform_model.h"

namespace VstCommon {

using WaveformParameter = GenericListParameter<WaveformDefinition>;

using WaveformSampleAccurateParameter = GenericListSampleAccurateParameter<WaveformDefinition>;

} // namespace VstCommon
