#include "cids.h"
#include "pids.h"
#include "public.sdk/source/vst/utility/audiobuffers.h"
#include "public.sdk/source/vst/utility/processdataslicer.h"
#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "base/source/fstreamer.h"
#include <array>
#include <cassert>
#include <limits>
#include <vector>

namespace Steinberg::Tutorial {

using namespace Steinberg::Vst;

struct state_model {
	double gain;
};

class gain_processor : public AudioEffect {
public:
	using rt_transfer = RTTransferT<state_model>;

	gain_processor();
	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
	tresult PLUGIN_API terminate() SMTG_OVERRIDE;
	tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 num_ins,
	                                       SpeakerArrangement* outputs,
	                                       int32 num_outs) SMTG_OVERRIDE;
	tresult PLUGIN_API canProcessSampleSize(int32 symbolic_sample_size) SMTG_OVERRIDE;
	tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE;

	void handle_parameter_changes(IParameterChanges* changes);

	template <SymbolicSampleSizes SampleSize>
	void process(ProcessData& data);

	SampleAccurate::Parameter gain_parameter {parameter_id::gain, 1.};
	rt_transfer state_transfer;
};

gain_processor::gain_processor()
{
	setControllerClass(controller_uid);
}

tresult PLUGIN_API gain_processor::initialize(FUnknown* context)
{
	auto result = AudioEffect::initialize(context);
	if (result == kResultTrue) {
		addAudioInput(STR("Input"), SpeakerArr::kStereo);
		addAudioOutput(STR("Output"), SpeakerArr::kStereo);
	}
	return result;
}

tresult PLUGIN_API gain_processor::terminate()
{
	state_transfer.clear_ui();
	return AudioEffect::terminate();
}

tresult PLUGIN_API gain_processor::setState(IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	IBStreamer streamer(state, kLittleEndian);

	uint32 num_params;
	if (streamer.readInt32u(num_params) == false)
		return kResultFalse;

	auto model = std::make_unique<state_model>();

	ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	model->gain = value;

	state_transfer.transferObject_ui(std::move(model));
	return kResultTrue;
}

tresult PLUGIN_API gain_processor::getState(IBStream* state)
{
	if (!state)
		return kInvalidArgument;

	IBStreamer streamer(state, kLittleEndian);
	streamer.writeDouble(gain_parameter.getValue());
	return kResultTrue;
}

tresult PLUGIN_API gain_processor::setBusArrangements(SpeakerArrangement* inputs, int32 num_ins,
                                                      SpeakerArrangement* outputs, int32 num_outs)
{
	if (num_ins != 1 || num_outs != 1)
		return kResultFalse;
	if (SpeakerArr::getChannelCount(inputs[0]) == SpeakerArr::getChannelCount(outputs[0])) {
		getAudioInput(0)->setArrangement(inputs[0]);
		getAudioOutput(0)->setArrangement(outputs[0]);
		return kResultTrue;
	}
	return kResultFalse;
}

tresult PLUGIN_API gain_processor::canProcessSampleSize(int32 symbolic_sample_size)
{
	return (symbolic_sample_size == SymbolicSampleSizes::kSample32 ||
	        symbolic_sample_size == SymbolicSampleSizes::kSample64) ?
	           kResultTrue :
	           kResultFalse;
}

template <SymbolicSampleSizes SampleSize>
void gain_processor::process(ProcessData& data)
{
	ProcessDataSlicer slicer(8);

	auto do_processing = [this](ProcessData& data) {
		ParamValue gain = gain_parameter.advance(data.numSamples);

		AudioBusBuffers* inputs = data.inputs;
		AudioBusBuffers* outputs = data.outputs;
		for (auto channel_index = 0; channel_index < inputs[0].numChannels; ++channel_index) {
			auto input_buffers = getChannelBuffers<SampleSize>(inputs[0])[channel_index];
			auto output_buffers = getChannelBuffers<SampleSize>(outputs[0])[channel_index];
			for (auto sample_index = 0; sample_index < data.numSamples; ++sample_index) {
				auto sample = input_buffers[sample_index];
				output_buffers[sample_index] = sample * gain;
			}
		}
	};

	slicer.process<SampleSize>(data, do_processing);
}

void gain_processor::handle_parameter_changes(IParameterChanges* changes)
{
	if (!changes)
		return;
	int32 change_count = changes->getParameterCount();
	for (auto i = 0; i < change_count; ++i) {
		if (auto queue = changes->getParameterData(i)) {
			auto param_id = queue->getParameterId();
			if (param_id == parameter_id::gain) {
				gain_parameter.beginChanges(queue);
			}
		}
	}
}

tresult PLUGIN_API gain_processor::process(ProcessData& data)
{
	state_transfer.accessTransferObject_rt(
	    [this](const auto& state_model) { gain_parameter.setValue(state_model.gain); });

	handle_parameter_changes(data.inputParameterChanges);

	if (processSetup.symbolicSampleSize == SymbolicSampleSizes::kSample32)
		process<SymbolicSampleSizes::kSample32>(data);
	else
		process<SymbolicSampleSizes::kSample64>(data);

	gain_parameter.endChanges();
	return kResultTrue;
}

FUnknown* create_processor_instance(void*)
{
	return static_cast<IAudioProcessor*>(new gain_processor);
}

}
