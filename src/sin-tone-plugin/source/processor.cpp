#include "base/source/fstreamer.h"
#include "cids.h"
#include "pids.h"
#include "pluginterfaces/base/ftypes.h"
#include "public.sdk/source/vst/utility/audiobuffers.h"
#include "public.sdk/source/vst/utility/processdataslicer.h"
#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <array>
#include <cassert>
#include <limits>
#include <set>
#include <vector>

namespace Steinberg::Tutorial {

struct state_model {
	double sin_freq = 440.0;
	double gain		= 0.25;
};

class sin_tone_processor : public Steinberg::Vst::AudioEffect {
  public:
	using rt_transfer = Steinberg::Vst::RTTransferT<state_model>;

	sin_tone_processor();
	Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *in_context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *in_state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *in_state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement *in_inputs, Steinberg::int32 in_num_ins,
													 Steinberg::Vst::SpeakerArrangement *in_outputs, Steinberg::int32 in_num_outs) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 in_symbolic_sample_size) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData &in_data) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup &setup) SMTG_OVERRIDE;

	void handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes);
	void update_phase_delta(Vst::SampleRate in_sample_rate);

	template <Steinberg::Vst::SymbolicSampleSizes SampleSize> void process(Steinberg::Vst::ProcessData &in_data);

	Steinberg::Vst::SampleAccurate::Parameter sin_freq_parameter{parameter_id::sin_freq_param, 1.};
	rt_transfer								  state_transfer;

	double current_phase = 0.0;
	double delta_phase	 = 0.0; // freq / sample_rate
};

sin_tone_processor::sin_tone_processor() {
	setControllerClass(controller_uid);
}

tresult PLUGIN_API sin_tone_processor::initialize(Steinberg::FUnknown *in_context) {
	auto result = Steinberg::Vst::AudioEffect::initialize(in_context);
	if (result == kResultTrue) {
		addAudioInput(STR("Input"), Steinberg::Vst::SpeakerArr::kStereo);
		addAudioOutput(STR("Output"), Steinberg::Vst::SpeakerArr::kStereo);
	}
	return result;
}

tresult PLUGIN_API sin_tone_processor::terminate() {
	state_transfer.clear_ui();
	return Steinberg::Vst::AudioEffect::terminate();
}

tresult PLUGIN_API sin_tone_processor::setState(IBStream *in_state) {
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

	model->sin_freq = value;

	state_transfer.transferObject_ui(std::move(model));
	return kResultTrue;
}

Steinberg::tresult PLUGIN_API sin_tone_processor::getState(Steinberg::IBStream *in_state) {
	if (!in_state)
		return kInvalidArgument;

	IBStreamer streamer(in_state, kLittleEndian);
	streamer.writeDouble(sin_freq_parameter.getValue());
	return kResultTrue;
}

tresult PLUGIN_API sin_tone_processor::setBusArrangements(Steinberg::Vst::SpeakerArrangement *in_inputs, int32 in_num_ins,
														  Steinberg::Vst::SpeakerArrangement *in_outputs, int32 in_num_outs) {
	if (in_num_ins != 1 || in_num_outs != 1)
		return kResultFalse;
	if (Steinberg::Vst::SpeakerArr::getChannelCount(in_inputs[0]) == Steinberg::Vst::SpeakerArr::getChannelCount(in_outputs[0])) {
		getAudioInput(0)->setArrangement(in_inputs[0]);
		getAudioOutput(0)->setArrangement(in_outputs[0]);
		return kResultTrue;
	}
	return kResultFalse;
}

tresult PLUGIN_API sin_tone_processor::canProcessSampleSize(int32 in_symbolic_sample_size) {
	return (in_symbolic_sample_size == Steinberg::Vst::SymbolicSampleSizes::kSample32 ||
			in_symbolic_sample_size == Steinberg::Vst::SymbolicSampleSizes::kSample64)
			   ? kResultTrue
			   : kResultFalse;
}

template <Steinberg::Vst::SymbolicSampleSizes SampleSize> void sin_tone_processor::process(Steinberg::Vst::ProcessData &in_data) {
	Steinberg::Vst::ProcessDataSlicer slicer(8);

	auto do_processing = [this](Steinberg::Vst::ProcessData &data) {
		Steinberg::Vst::ParamValue sin_tone = sin_freq_parameter.advance(data.numSamples);

		Steinberg::Vst::AudioBusBuffers *inputs	 = data.inputs;
		Steinberg::Vst::AudioBusBuffers *outputs = data.outputs;
		for (auto channel_index = 0; channel_index < inputs[0].numChannels; ++channel_index) {
			auto input_buffers	= Steinberg::Vst::getChannelBuffers<SampleSize>(inputs[0])[channel_index];
			auto output_buffers = Steinberg::Vst::getChannelBuffers<SampleSize>(outputs[0])[channel_index];
			for (auto sample_index = 0; sample_index < data.numSamples; ++sample_index) {
				auto sample					 = input_buffers[sample_index];
				output_buffers[sample_index] = sample * sin_tone;
			}
		}
	};

	slicer.process<SampleSize>(in_data, do_processing);
}

void sin_tone_processor::handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes) {
	if (!in_changes)
		return;
	int32 change_count = in_changes->getParameterCount();
	for (auto i = 0; i < change_count; ++i) {
		if (auto queue = in_changes->getParameterData(i)) {
			auto param_id = queue->getParameterId();
			if (param_id == parameter_id::sin_freq_param) {
				sin_freq_parameter.beginChanges(queue);
			}
		}
	}
}

tresult PLUGIN_API sin_tone_processor::process(Steinberg::Vst::ProcessData &in_data) {
	state_transfer.accessTransferObject_rt([this](const auto &state_model) { sin_freq_parameter.setValue(state_model.sin_freq); });

	handle_parameter_changes(in_data.inputParameterChanges);

	if (processSetup.symbolicSampleSize == Steinberg::Vst::SymbolicSampleSizes::kSample32)
		process<Steinberg::Vst::SymbolicSampleSizes::kSample32>(in_data);
	else
		process<Steinberg::Vst::SymbolicSampleSizes::kSample64>(in_data);

	sin_freq_parameter.endChanges();
	return kResultTrue;
}

Steinberg::tresult PLUGIN_API sin_tone_processor::setupProcessing(Steinberg::Vst::ProcessSetup &setup) {
	update_phase_delta(setup.sampleRate);
	return kResultTrue;
}

void sin_tone_processor::update_phase_delta(Vst::SampleRate in_sample_rate) {
	delta_phase = sin_freq_parameter.getValue() / in_sample_rate;
}

FUnknown *create_processor_instance(void *in_ptr) {
	return static_cast<Steinberg::Vst::IAudioProcessor *>(new sin_tone_processor);
}

} // namespace Steinberg::Tutorial
