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

struct state_model {
	double gain;
};

class gain_processor : public Steinberg::Vst::AudioEffect {
public:
	using rt_transfer = Steinberg::Vst::RTTransferT<state_model>;

	gain_processor();
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* in_context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* in_state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* in_state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* in_inputs, Steinberg::int32 in_num_ins,
                                 Steinberg::Vst::SpeakerArrangement* in_outputs,
                                 Steinberg::int32 in_num_outs) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 in_symbolic_sample_size) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& in_data) SMTG_OVERRIDE;

    void handle_parameter_changes(Steinberg::Vst::IParameterChanges* in_changes);

	template <Steinberg::Vst::SymbolicSampleSizes SampleSize>
	void process(Steinberg::Vst::ProcessData& in_data);

	Steinberg::Vst::SampleAccurate::Parameter gain_parameter {parameter_id::gain, 1.};
	 rt_transfer state_transfer;
};

gain_processor::gain_processor() {
	setControllerClass(controller_uid);
}

tresult PLUGIN_API gain_processor::initialize(Steinberg::FUnknown* in_context) {
	auto result = Steinberg::Vst::AudioEffect::initialize(in_context);
	if (result == kResultTrue) {
		addAudioInput(STR("Input"), Steinberg::Vst::SpeakerArr::kStereo);
		addAudioOutput(STR("Output"), Steinberg::Vst::SpeakerArr::kStereo);
	}
	return result;
}

tresult PLUGIN_API gain_processor::terminate() {
	state_transfer.clear_ui();
	return Steinberg::Vst::AudioEffect::terminate();
}

tresult PLUGIN_API gain_processor::setState(IBStream* in_state) {
	if (!in_state)
		return kInvalidArgument;

	IBStreamer streamer(in_state, kLittleEndian);

	uint32 num_params;
	if (streamer.readInt32u(num_params) == false)
		return kResultFalse;

	auto model = std::make_unique<state_model>();

    Steinberg::Vst::ParamValue value;
	if (!streamer.readDouble(value))
		return kResultFalse;

	model->gain = value;

    state_transfer.transferObject_ui(std::move(model));
    return kResultTrue;
}

Steinberg::tresult PLUGIN_API gain_processor::getState(Steinberg::IBStream* in_state) {
    if (!in_state)
        return kInvalidArgument;

    IBStreamer streamer(in_state, kLittleEndian);
    streamer.writeDouble(gain_parameter.getValue());
    return kResultTrue;
}

tresult PLUGIN_API gain_processor::setBusArrangements(Steinberg::Vst::SpeakerArrangement* in_inputs, int32 in_num_ins,
                                 Steinberg::Vst::SpeakerArrangement* in_outputs, int32 in_num_outs) {
	if (in_num_ins != 1 || in_num_outs != 1)
		return kResultFalse;
    if (Steinberg::Vst::SpeakerArr::getChannelCount(in_inputs[0]) == Steinberg::Vst::SpeakerArr::getChannelCount(in_outputs[0])) {
	getAudioInput(0)->setArrangement(in_inputs[0]);
	getAudioOutput(0)->setArrangement(in_outputs[0]);
	return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API gain_processor::canProcessSampleSize(int32 in_symbolic_sample_size) {
	return (in_symbolic_sample_size == Steinberg::Vst::SymbolicSampleSizes::kSample32 ||
		in_symbolic_sample_size == Steinberg::Vst::SymbolicSampleSizes::kSample64) ?
			kResultTrue :
			kResultFalse;
}

template <Steinberg::Vst::SymbolicSampleSizes SampleSize>
void gain_processor::process(Steinberg::Vst::ProcessData& in_data) {
	Steinberg::Vst::ProcessDataSlicer slicer(8);

	auto do_processing = [this](Steinberg::Vst::ProcessData& data) {
		Steinberg::Vst::ParamValue gain = gain_parameter.advance(data.numSamples);

		Steinberg::Vst::AudioBusBuffers* inputs = data.inputs;
		Steinberg::Vst::AudioBusBuffers* outputs = data.outputs;
		for (auto channel_index = 0; channel_index < inputs[0].numChannels; ++channel_index) {
			auto input_buffers = Steinberg::Vst::getChannelBuffers<SampleSize>(inputs[0])[channel_index];
			auto output_buffers = Steinberg::Vst::getChannelBuffers<SampleSize>(outputs[0])[channel_index];
			for (auto sample_index = 0; sample_index < data.numSamples; ++sample_index) {
				auto sample = input_buffers[sample_index];
				output_buffers[sample_index] = sample * gain;
			}
		}
	};

	slicer.process<SampleSize>(in_data, do_processing);
}

void gain_processor::handle_parameter_changes(Steinberg::Vst::IParameterChanges* in_changes) {
	if (!in_changes)
		return;
	int32 change_count = in_changes->getParameterCount();
	for (auto i = 0; i < change_count; ++i) {
		if (auto queue = in_changes->getParameterData(i)) {
			auto param_id = queue->getParameterId();
			if (param_id == parameter_id::gain) {
				gain_parameter.beginChanges(queue);
			}
		}
	}
}

tresult PLUGIN_API gain_processor::process(Steinberg::Vst::ProcessData& in_data) {
	state_transfer.accessTransferObject_rt(
		[this](const auto& state_model) { gain_parameter.setValue(state_model.gain); });

	handle_parameter_changes(in_data.inputParameterChanges);

	if (processSetup.symbolicSampleSize == Steinberg::Vst::SymbolicSampleSizes::kSample32)
		process<Steinberg::Vst::SymbolicSampleSizes::kSample32>(in_data);
	else
		process<Steinberg::Vst::SymbolicSampleSizes::kSample64>(in_data);

	gain_parameter.endChanges();
	return kResultTrue;
}

FUnknown* create_processor_instance(void* in_ptr) {
	return static_cast<Steinberg::Vst::IAudioProcessor*>(new gain_processor);
}

}
