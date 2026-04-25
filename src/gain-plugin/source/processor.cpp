//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace GainVendor {
//------------------------------------------------------------------------
// GainProcessor
//------------------------------------------------------------------------
GainProcessor::GainProcessor() {
	//--- set the wanted controller for our processor
	setControllerClass(kGainControllerUID);
}

//------------------------------------------------------------------------
GainProcessor::~GainProcessor() {}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::initialize(FUnknown *context) {
	// Here the Plug-in will be instantiated

	//---always initialize the parent-------
	tresult result = AudioEffect::initialize(context);
	// if everything Ok, continue
	if (result != kResultOk) {
		return result;
	}

	//--- create Audio IO ------
	addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput(STR16("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	gain_rt.clear_ui();

	//---do not forget to call parent ------
	return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::setActive(TBool state) {
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::process(Vst::ProcessData &data) {
	gain_rt.accessTransferObject_rt([this](const auto &state_model) { gain_parameter.setValue(state_model.gain_level); });
	handle_parameter_changes(data.inputParameterChanges);
	//--- First : Read inputs parameter changes-----------

	/*if (data.inputParameterChanges)
	{
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
		for (int32 index = 0; index < numParamsChanged; index++)
		{
			if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
			{
				Vst::ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount ();
				switch (paramQueue->getParameterId ())
				{
				}
			}
		}
	}*/

	//--- Here you have to implement your processing

	if (data.numSamples > 0) {
		//--- ------------------------------------------
		// here as example a default implementation where we try to copy the inputs to the outputs:
		// if less input than outputs then clear outputs
		//--- ------------------------------------------
		Steinberg::Vst::ParamValue gain = gain_parameter.advance(data.numSamples);

		int32 minBus = std::min(data.numInputs, data.numOutputs);
		for (int32 i = 0; i < minBus; i++) {
			int32 minChan = std::min(data.inputs[i].numChannels, data.outputs[i].numChannels);
			for (int32 c = 0; c < minChan; c++) {
				// do not need to be copied if the buffers are the same
				if (data.outputs[i].channelBuffers32[c] != data.inputs[i].channelBuffers32[c]) {
					for (auto sample_index = 0; sample_index < data.numSamples; ++sample_index)
						data.outputs[i].channelBuffers32[c][sample_index] = data.inputs[i].channelBuffers32[c][sample_index] * gain;
				}
			}
			data.outputs[i].silenceFlags = data.inputs[i].silenceFlags;

			// clear the remaining output buffers
			for (int32 c = minChan; c < data.outputs[i].numChannels; c++) {
				// clear output buffers
				memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));

				// inform the host that this channel is silent
				data.outputs[i].silenceFlags |= ((uint64)1 << c);
			}
		}
		// clear the remaining output buffers
		for (int32 i = minBus; i < data.numOutputs; i++) {
			// clear output buffers
			for (int32 c = 0; c < data.outputs[i].numChannels; c++) {
				memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));
			}
			// inform the host that this bus is silent
			data.outputs[i].silenceFlags = ((uint64)1 << data.outputs[i].numChannels) - 1;
		}
	}

	gain_parameter.endChanges();
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
	//--- called before any processing ----
	return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::setState(IBStream *state) {
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer(state, kLittleEndian);

	Steinberg::Vst::ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	auto gain_rt_tmp		= std::make_unique<gain_param>();
	gain_rt_tmp->gain_level = value;
	gain_rt.transferObject_ui(std::move(gain_rt_tmp));

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API GainProcessor::getState(IBStream *state) {
	// here we need to save the model
	IBStreamer streamer(state, kLittleEndian);
	streamer.writeDouble(gain_parameter.getValue());

	return kResultOk;
}

//------------------------------------------------------------------------

void GainProcessor::handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes) {
	if (!in_changes)
		return;
	int32 change_count = in_changes->getParameterCount();
	for (auto i = 0; i < change_count; ++i) {
		if (auto queue = in_changes->getParameterData(i)) {
			auto param_id = queue->getParameterId();
			if (param_id == GainVendor::gain_param.id) {
				gain_parameter.beginChanges(queue);
			}
		}
	}
}

} // namespace GainVendor
