//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "controller.h"
#include "base/source/fstreamer.h"
#include "vst_common/frequency_model.h"
#include "vst_common/frequency_parameter.h"
#include "vst_common/waveform_model.h"
#include "vst_common/waveform_parameter.h"
#include "vst_common/volume_parameter.h"

using namespace Steinberg;

namespace ToneGeneratorVendor {

//------------------------------------------------------------------------
// ToneGeneratorController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorController::initialize(FUnknown *context) {
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize(context);
	if (result != kResultOk) {
		return result;
	}

	// Here you could register some parameters
	parameters.addParameter(new VstCommon::FrequencyParameter());
	parameters.addParameter(new VstCommon::VolumeParameter());
	parameters.addParameter(new VstCommon::WaveformParameter());
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorController::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorController::setComponentState(IBStream *state) {
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	Steinberg::IBStreamer streamer(state, kLittleEndian);

	Steinberg::Vst::ParamValue freq_value = 0.0;
	Steinberg::Vst::ParamValue vol_value = VstCommon::VolumeModel::toNormalized(VstCommon::VolumeModel::kDefaultPlain);
	Steinberg::Vst::ParamValue waveform_value = VstCommon::WaveformModel::toNormalized(VstCommon::WaveformModel::kDefaultPlain);
	if (!streamer.readDouble(freq_value))
		return kResultFalse;

	// Try to read volume (backward compatibility with old states that only had frequency)
	streamer.readDouble(vol_value);

	// Try to read waveform (backward compatibility with states that only had frequency + volume)
	streamer.readDouble(waveform_value);

	if (auto param = parameters.getParameter(VstCommon::FrequencyModel::kId))
		param->setNormalized(freq_value);
	if (auto param = parameters.getParameter(VstCommon::VolumeModel::kId))
		param->setNormalized(vol_value);
	if (auto param = parameters.getParameter(VstCommon::WaveformModel::kId))
		param->setNormalized(waveform_value);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorController::setState(IBStream *state) {
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorController::getState(IBStream *state) {
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView *PLUGIN_API ToneGeneratorController::createView(FIDString name) {
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
		// create your editor here and return a IPlugView ptr of it
		return nullptr;
	}
	return nullptr;
}

//------------------------------------------------------------------------
} // namespace ToneGeneratorVendor
