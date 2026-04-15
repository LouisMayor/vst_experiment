#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/vstparameters.h"

class FrequencyParameter : public Steinberg::Vst::Parameter {
  public:
	FrequencyParameter() = default;
	FrequencyParameter(const Steinberg::Vst::TChar *in_title, Steinberg::Vst::ParamID in_tag);

	static Steinberg::Vst::ParamValue get_plain(Steinberg::Vst::ParamValue in_norm) {
		return in_norm * MAX_FREQ;
	}

	// plain/norm ops
	Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue in_norm) const SMTG_OVERRIDE;
	Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue in_plain) const SMTG_OVERRIDE;

	// string ops
	void toString(Steinberg::Vst::ParamValue in_norm, Steinberg::Vst::String128 string) const SMTG_OVERRIDE;
	bool fromString(const Steinberg::Vst::TChar *string, Steinberg::Vst::ParamValue &valueNormalized) const SMTG_OVERRIDE;

	struct DSP : public Steinberg::Vst::SampleAccurate::Parameter {
		Steinberg::Vst::Sample64 get_freq(Steinberg::Vst::ParamValue in_norm) {
			return get_plain(in_norm);
		}
	};

  protected:
	static constexpr double DEFAULT	 = 0.5;
	static constexpr double MAX_FREQ = 1000.0; // 1000hz
};
