#include "freq_param.h"
#include "pluginterfaces/base/fstrdefs.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <cstdlib>

FrequencyParameter::FrequencyParameter(const Steinberg::Vst::TChar *in_title, Steinberg::Vst::ParamID in_tag)
	: Steinberg::Vst::Parameter(in_title, in_tag, STR16("Hz"), DEFAULT, 0) {}

Steinberg::Vst::ParamValue FrequencyParameter::toPlain(Steinberg::Vst::ParamValue in_norm) const {
	return get_plain(in_norm);
}

Steinberg::Vst::ParamValue FrequencyParameter::toNormalized(Steinberg::Vst::ParamValue in_plain) const {
	return in_plain / MAX_FREQ;
}

void FrequencyParameter::toString(Steinberg::Vst::ParamValue in_norm, Steinberg::Vst::String128 string) const {
	Steinberg::Vst::ParamValue hz = toPlain(in_norm);

	char text[32];
	if (hz >= 1000.0)
		snprintf(text, sizeof(text), "%.2f kHz", hz / 1000.0);
	else
		snprintf(text, sizeof(text), "%.0f Hz", hz);

	char norm_txt[32];
	snprintf(norm_txt, sizeof(norm_txt), "%.2f", in_norm);

	Steinberg::UString fmt_str = USTRING(text).append(USTRING(" | ").append(USTRING(norm_txt)));
	Steinberg::UString(string, 128).append(fmt_str);
}

bool FrequencyParameter::fromString(const Steinberg::Vst::TChar *in_string, Steinberg::Vst::ParamValue &out_value_norm) const {
	Steinberg::UString ustr = Steinberg::UString((Steinberg::Vst::TChar *)in_string, -1);
	double			   value;
	ustr.scanFloat(value);
	out_value_norm = toNormalized(value);
	return true;
}
