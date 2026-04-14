#include "base/source/fstreamer.h"
#include "pids.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace Steinberg::Tutorial {

constexpr double MAX_FREQ	  = 1000.0;
constexpr double DEFAULT_GAIN = 0.25;

class controller : public Steinberg::Vst::EditController {
  public:
	tresult PLUGIN_API initialize(FUnknown *in_context) SMTG_OVERRIDE;
	tresult PLUGIN_API setComponentState(Steinberg::IBStream *in_state) SMTG_OVERRIDE;
};

tresult PLUGIN_API controller::initialize(FUnknown *in_context) {
	tresult result = Steinberg::Vst::EditController::initialize(in_context);
	if (result != kResultOk) {
		return result;
	}
	parameters.addParameter(STR("Sin Freq"), STR("Hz"), 0, 1., Steinberg::Vst::ParameterInfo::kCanAutomate, parameter_id::sin_freq_param);
	parameters.addParameter(STR("Gain"), STR("%"), 0, DEFAULT_GAIN, Steinberg::Vst::ParameterInfo::kCanAutomate, parameter_id::gain_param);
	return kResultOk;
}

tresult PLUGIN_API controller::setComponentState(Steinberg::IBStream *state) {
	if (!state)
		return kInvalidArgument;

	Steinberg::IBStreamer streamer(state, kLittleEndian);

	Steinberg::Vst::ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	if (auto param = parameters.getParameter(parameter_id::sin_freq_param))
		param->setNormalized(value);

	if (auto param = parameters.getParameter(parameter_id::gain_param))
		param->setNormalized(value);
	return kResultTrue;
}

FUnknown *create_controller_instance(void *) {
	return static_cast<Steinberg::Vst::IEditController *>(new controller());
}

} // namespace Steinberg::Tutorial
