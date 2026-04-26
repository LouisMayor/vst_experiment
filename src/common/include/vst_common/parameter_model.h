#pragma once

namespace VstCommon {

/**
 * ParameterModel Contract
 *
 * A ParameterModel is a stateless traits struct that defines all metadata and
 * conversion logic for a single parameter. It is consumed by the template
 * adapters ModelRangeParameter and ModelSampleAccurateParameter.
 *
 * Required static members:
 *   - kId                  : Steinberg::Vst::ParamID
 *   - kTitle               : const Steinberg::Vst::TChar*
 *   - kUnits               : const Steinberg::Vst::TChar*
 *   - kMinPlain            : Steinberg::Vst::ParamValue
 *   - kMaxPlain            : Steinberg::Vst::ParamValue
 *   - kDefaultPlain        : Steinberg::Vst::ParamValue
 *   - kFlags               : Steinberg::int32
 *
 * Optional static members:
 *   - kStepCount           : Steinberg::int32 (for discrete/list parameters)
 *
 * Required static methods:
 *   - toPlain(norm)        : Steinberg::Vst::ParamValue
 *   - toNormalized(plain)  : Steinberg::Vst::ParamValue
 *   - toString(norm, str)  : void
 *   - fromString(str, norm) : bool
 *
 * Models are unit-agnostic: kMinPlain/kMaxPlain can represent Hz, dB, ms, %, etc.
 * The kUnits string should reflect the human-readable unit (e.g. STR16("Hz")).
 */
struct ParameterModel {
	// This struct exists purely for documentation purposes.
	// Concrete models should follow the contract described above.
};

} // namespace VstCommon
