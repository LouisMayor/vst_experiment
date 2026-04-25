//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "controller.h"
#include "base/source/fstreamer.h"
#include "cids.h"
#include "pid.h"

using namespace Steinberg;

namespace GainVendor {

//------------------------------------------------------------------------
// GainController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API GainController::initialize(FUnknown *context) {
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize(context);
	if (result != kResultOk) {
		return result;
	}

	// Here you could register some parameters
	parameters.addParameter(GainVendor::gain_param.title, GainVendor::gain_param.units, GainVendor::gain_param.step_count,
							GainVendor::gain_param.default_normalized_value, GainVendor::gain_param.flags, GainVendor::gain_param.id);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainController::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainController::setComponentState(IBStream *state) {
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	Steinberg::IBStreamer streamer(state, kLittleEndian);

	Steinberg::Vst::ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	if (auto param = parameters.getParameter(GainVendor::gain_param.id))
		param->setNormalized(value);
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainController::setState(IBStream *state) {
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainController::getState(IBStream *state) {
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView *PLUGIN_API GainController::createView(FIDString name) {
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
		// create your editor here and return a IPlugView ptr of it
		return nullptr;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace GainVendor
