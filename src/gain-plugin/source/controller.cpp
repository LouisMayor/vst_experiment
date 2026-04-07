#include "pids.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "base/source/fstreamer.h"

namespace Steinberg::Tutorial {

using namespace Steinberg::Vst;

class controller : public EditController {
public:
	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API setComponentState(IBStream* state) SMTG_OVERRIDE;
};

tresult PLUGIN_API controller::initialize(FUnknown* context)
{
	tresult result = EditController::initialize(context);
	if (result != kResultOk) {
		return result;
	}
	parameters.addParameter(STR("Gain"), STR("%"), 0, 1., ParameterInfo::kCanAutomate,
	                       parameter_id::gain);
	return kResultOk;
}

tresult PLUGIN_API controller::setComponentState(IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	IBStreamer streamer(state, kLittleEndian);

	ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	if (auto param = parameters.getParameter(parameter_id::gain))
		param->setNormalized(value);
	return kResultTrue;
}

FUnknown* create_controller_instance(void*)
{
	return static_cast<IEditController*>(new controller);
}

}
