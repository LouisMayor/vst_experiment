//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "processor.h"
#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "vst_common/frequency_model.h"
#include "vst_common/volume_model.h"
#include "vst_common/waveform_model.h"

#include <cmath>

using namespace Steinberg;

namespace ToneGeneratorVendor {
//------------------------------------------------------------------------
// ToneGeneratorProcessor
//------------------------------------------------------------------------
ToneGeneratorProcessor::ToneGeneratorProcessor() {
	//--- set the wanted controller for our processor
	setControllerClass(kToneGeneratorControllerUID);
}

//------------------------------------------------------------------------
ToneGeneratorProcessor::~ToneGeneratorProcessor() {}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::initialize(FUnknown *context) {
	// Here the Plug-in will be instantiated

	//---always initialize the parent-------
	tresult result = AudioEffect::initialize(context);
	// if everything Ok, continue
	if (result != kResultOk) {
		return result;
	}

	//--- create Audio IO ------
	// addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput(STR16("Event In"), 1);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::terminate() {
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::setActive(TBool state) {
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::process(Vst::ProcessData &data) {
	freq_rt.accessTransferObject_rt([this](const auto &state_model) { freq_param.setValue(state_model.freq); });
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
		Steinberg::Vst::ParamValue freq_norm	  = freq_param.advance(data.numSamples);
		Steinberg::Vst::ParamValue freq_hz		  = VstCommon::FrequencyModel::toPlain(freq_norm);
		Steinberg::Vst::ParamValue vol_norm		  = vol_param.advance(data.numSamples);
		Steinberg::Vst::ParamValue gain			  = VstCommon::VolumeModel::toGain(vol_norm);
		Steinberg::Vst::ParamValue waveform_norm  = waveform_param.advance(data.numSamples);
		std::size_t				   waveform_index = VstCommon::WaveformModel::toIndex(waveform_norm);
		constexpr double		   two_pi		  = 2.0 * 3.14159265358979323846;

		auto get_tone = [this, freq_hz, waveform_index] {
			auto tone		 = Vst::Sample32(0.0f);
			auto phase_ratio = phase / two_pi;

			switch (waveform_index) {
			case 1:
				tone = static_cast<Vst::Sample32>((2.0 * phase_ratio) - 1.0);
				break;
			case 2:
				tone = (phase_ratio < 0.5) ? Vst::Sample32(1.0f) : Vst::Sample32(-1.0f);
				break;
			case 0:
			default:
				tone = static_cast<Vst::Sample32>(std::sin(phase));
				break;
			}

			phase += two_pi * freq_hz / processSetup.sampleRate;
			if (phase >= two_pi)
				phase -= two_pi;
			return tone;
		};

		for (int32 s = 0; s < data.numSamples; ++s) {
			auto tone = get_tone() * gain;
			for (int32 i = 0; i < data.numOutputs; i++) {
				for (int32 c = 0; c < data.outputs[i].numChannels; c++)
					data.outputs[i].channelBuffers32[c][s] = tone;
			}
		}
	}

	freq_param.endChanges();
	vol_param.endChanges();
	waveform_param.endChanges();
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
	//--- called before any processing ----
	return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::setState(IBStream *state) {
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer(state, kLittleEndian);

	Steinberg::Vst::ParamValue freq_value	  = 0.0;
	Steinberg::Vst::ParamValue vol_value	  = VstCommon::VolumeModel::toNormalized(VstCommon::VolumeModel::kDefaultPlain);
	Steinberg::Vst::ParamValue waveform_value = VstCommon::WaveformModel::toNormalized(VstCommon::WaveformModel::kDefaultPlain);

	if (streamer.readDouble(freq_value))
		freq_param.setValue(freq_value);

	// Try to read volume (backward compatibility with old states that only had frequency)
	if (streamer.readDouble(vol_value))
		vol_param.setValue(vol_value);

	// Try to read waveform (backward compatibility with states that only had frequency + volume)
	if (streamer.readDouble(waveform_value))
		waveform_param.setValue(waveform_value);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ToneGeneratorProcessor::getState(IBStream *state) {
	// here we need to save the model
	IBStreamer streamer(state, kLittleEndian);
	streamer.writeDouble(freq_param.getValue());
	streamer.writeDouble(vol_param.getValue());
	streamer.writeDouble(waveform_param.getValue());
	return kResultOk;
}

//------------------------------------------------------------------------
void ToneGeneratorProcessor::handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes) {
	if (!in_changes)
		return;
	int32 change_count = in_changes->getParameterCount();
	for (auto i = 0; i < change_count; ++i) {
		if (auto queue = in_changes->getParameterData(i)) {
			auto param_id = queue->getParameterId();
			if (param_id == freq_param.getParamID()) {
				freq_param.beginChanges(queue);
			}
			if (param_id == vol_param.getParamID()) {
				vol_param.beginChanges(queue);
			}
			if (param_id == waveform_param.getParamID()) {
				waveform_param.beginChanges(queue);
			}
		}
	}
}

//------------------------------------------------------------------------
} // namespace ToneGeneratorVendor
