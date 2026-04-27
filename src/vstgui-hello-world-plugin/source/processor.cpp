//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <cmath>

using namespace Steinberg;

namespace VstguiHelloWorldVendor {
//------------------------------------------------------------------------
// VstguiHelloWorldProcessor
//------------------------------------------------------------------------
VstguiHelloWorldProcessor::VstguiHelloWorldProcessor() {
	//--- set the wanted controller for our processor
	setControllerClass(kVstguiHelloWorldControllerUID);
}

//------------------------------------------------------------------------
VstguiHelloWorldProcessor::~VstguiHelloWorldProcessor() {}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::initialize(FUnknown *context) {
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
tresult PLUGIN_API VstguiHelloWorldProcessor::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::setActive(TBool state) {
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::process(Vst::ProcessData &data) {
	//--- First : Read inputs parameter changes-----------
	float gain = mGain;
	if (data.inputParameterChanges) {
		int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
		for (int32 index = 0; index < numParamsChanged; index++) {
			if (auto *paramQueue = data.inputParameterChanges->getParameterData(index)) {
				Vst::ParamValue value;
				int32			sampleOffset;
				int32			numPoints = paramQueue->getPointCount();
				switch (paramQueue->getParameterId()) {
				case kGainId:
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
						// Map normalized 0..1 to a useful gain range:
						// value 0 -> 0.0 (silence)
						// value 0.5 -> 1.0 (unity)
						// value 1.0 -> 2.0 (double)
						mGain = static_cast<float>(value) * 2.f;
						gain = mGain;
					}
					break;
				}
			}
		}
	}

	//--- Here you have to implement your processing

	if (data.numSamples > 0) {
		//--- ------------------------------------------
		// here as example a default implementation where we try to copy the inputs to the outputs:
		// if less input than outputs then clear outputs
		//--- ------------------------------------------

		int32 minBus = std::min(data.numInputs, data.numOutputs);
		for (int32 i = 0; i < minBus; i++) {
			int32 minChan = std::min(data.inputs[i].numChannels, data.outputs[i].numChannels);
			for (int32 c = 0; c < minChan; c++) {
				if (data.symbolicSampleSize == Vst::kSample32) {
					// Apply gain to 32-bit samples
					Vst::Sample32 *in  = data.inputs[i].channelBuffers32[c];
					Vst::Sample32 *out = data.outputs[i].channelBuffers32[c];
					for (int32 s = 0; s < data.numSamples; s++) {
						out[s] = in[s] * gain;
					}
				} else if (data.symbolicSampleSize == Vst::kSample64) {
					// Apply gain to 64-bit samples
					Vst::Sample64 *in  = data.inputs[i].channelBuffers64[c];
					Vst::Sample64 *out = data.outputs[i].channelBuffers64[c];
					for (int32 s = 0; s < data.numSamples; s++) {
						out[s] = in[s] * gain;
					}
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

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
	//--- called before any processing ----
	return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::setState(IBStream *state) {
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer(state, kLittleEndian);

	float savedGain = 0.f;
	if (streamer.readFloat(savedGain)) {
		mGain = savedGain;
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API VstguiHelloWorldProcessor::getState(IBStream *state) {
	// here we need to save the model
	IBStreamer streamer(state, kLittleEndian);

	streamer.writeFloat(mGain);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace VstguiHelloWorldVendor
