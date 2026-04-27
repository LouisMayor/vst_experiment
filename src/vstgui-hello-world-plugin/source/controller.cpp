//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "controller.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"

using namespace Steinberg;
using namespace VSTGUI;

namespace VstguiHelloWorldVendor {

//------------------------------------------------------------------------
// VstguiHelloWorldController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::initialize(FUnknown *context) {
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize(context);
	if (result != kResultOk) {
		return result;
	}

	// Here you could register some parameters
	//---Gain parameter--
	parameters.addParameter(STR16("Gain"),					  // title
							STR16("dB"),					  // units
							0,								  // stepCount
							0.5,							  // defaultNormalizedValue
							Vst::ParameterInfo::kCanAutomate, // flags
							kGainId,						  // tag
							0,								  // unitID (root)
							STR16("Gain"));					  // shortTitle

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::setComponentState(IBStream *state) {
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);
	float	   savedGain = 0.f;
	if (streamer.readFloat(savedGain) == false)
		return kResultFalse;
	setParamNormalized(kGainId, savedGain);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::setState(IBStream *state) {
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::getState(IBStream *state) {
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView *PLUGIN_API VstguiHelloWorldController::createView(FIDString name) {
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
		// create your editor here and return a IPlugView ptr of it
		auto *view = new VST3Editor(this, "view", "editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value) {
	// called from host to update our parameters state
	tresult result = EditControllerEx1::setParamNormalized(tag, value);
	return result;
}

//------------------------------------------------------------------------
IController *VstguiHelloWorldController::createSubController(UTF8StringPtr /*name*/, const IUIDescription * /*description*/, VST3Editor * /*editor*/) {
	// Sub-controllers can be created here if needed
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace VstguiHelloWorldVendor
