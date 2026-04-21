#include "base/source/fstreamer.h"
#include "cids.h"
#include "params/freq_param.h"
#include "pids.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/utility/audiobuffers.h"
#include "public.sdk/source/vst/utility/processdataslicer.h"
#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <cassert>
#include <cmath>

namespace Steinberg::Tutorial {

static constexpr double MAX_FREQ	 = 1000.0;
static constexpr double DEFAULT_FREQ = 440.0;
static constexpr double DEFAULT_GAIN = 0.25;
static constexpr double T_PI		 = 2.0 * 3.141592653589793;

struct state_model {
	double sin_freq;
	double gain;
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

	void		  handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes);
	void		  on_sample_rate_changed(Vst::SampleRate in_sample_rate);
	void		  update_phase_delta(Vst::Sample64 in_freq, Vst::SampleRate in_sample_rate);
	Vst::Sample64 get_sine();

	template <Steinberg::Vst::SymbolicSampleSizes SampleSize> void process(Steinberg::Vst::ProcessData &in_data);

	FrequencyParameter::DSP					  sin_freq_parameter{parameter_id::sin_freq_param};
	Steinberg::Vst::SampleAccurate::Parameter gain_parameter{parameter_id::gain_param, DEFAULT_GAIN};
	rt_transfer								  state_transfer;

	Vst::SampleRate current_sample_rate = 0.0;
	double			current_phase		= 0.0;
	double			delta_phase			= 0.0; // freq / sample_rate
};

sin_tone_processor::sin_tone_processor() {
	setControllerClass(controller_uid);
}

tresult PLUGIN_API sin_tone_processor::initialize(Steinberg::FUnknown *in_context) {
	auto result = Steinberg::Vst::AudioEffect::initialize(in_context);
	if (result == kResultTrue) {
		addAudioOutput(STR("Output"), Steinberg::Vst::SpeakerArr::kStereo);
		addEventInput(STR("Event In"), 1);
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
	model->sin_freq = value * MAX_FREQ;

	if (!streamer.readDouble(value))
		return kResultFalse;
	model->gain = value;

	state_transfer.transferObject_ui(std::move(model));
	return kResultTrue;
}

Steinberg::tresult PLUGIN_API sin_tone_processor::getState(Steinberg::IBStream *in_state) {
	if (!in_state)
		return kInvalidArgument;

	IBStreamer streamer(in_state, kLittleEndian);
	streamer.writeDouble(sin_freq_parameter.getValue());
	streamer.writeDouble(gain_parameter.getValue());
	return kResultTrue;
}

tresult PLUGIN_API sin_tone_processor::setBusArrangements(Steinberg::Vst::SpeakerArrangement *in_inputs, int32 in_num_ins,
														  Steinberg::Vst::SpeakerArrangement *in_outputs, int32 in_num_outs) {
	if (in_num_outs != 1)
		return kResultFalse;

	getAudioOutput(0)->setArrangement(in_outputs[0]);
	return kResultTrue;
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
		Steinberg::Vst::ParamValue gain		= gain_parameter.advance(data.numSamples);
		update_phase_delta(sin_freq_parameter.get_freq(sin_tone), current_sample_rate);

		Steinberg::Vst::AudioBusBuffers *outputs		 = data.outputs;
		auto							 channel_buffers = Steinberg::Vst::getChannelBuffers<SampleSize>(outputs[0]);
		for (auto sample_index = 0; sample_index < data.numSamples; ++sample_index) {
			auto sample = get_sine() * gain;
			for (auto channel_index = 0; channel_index < outputs[0].numChannels; ++channel_index) {
				channel_buffers[channel_index][sample_index] = sample;
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
			} else if (param_id == parameter_id::gain_param) {
				gain_parameter.beginChanges(queue);
			}
		}
	}
}

tresult PLUGIN_API sin_tone_processor::process(Steinberg::Vst::ProcessData &in_data) {
	state_transfer.accessTransferObject_rt([this](const auto &state_model) {
		sin_freq_parameter.setValue(state_model.sin_freq);
		gain_parameter.setValue(state_model.gain);
	});

	handle_parameter_changes(in_data.inputParameterChanges);

	if (processSetup.symbolicSampleSize == Steinberg::Vst::SymbolicSampleSizes::kSample32)
		process<Steinberg::Vst::SymbolicSampleSizes::kSample32>(in_data);
	else
		process<Steinberg::Vst::SymbolicSampleSizes::kSample64>(in_data);

	sin_freq_parameter.endChanges();
	gain_parameter.endChanges();
	return kResultTrue;
}

Steinberg::tresult PLUGIN_API sin_tone_processor::setupProcessing(Steinberg::Vst::ProcessSetup &setup) {
	on_sample_rate_changed(setup.sampleRate);
	return kResultTrue;
}

void sin_tone_processor::on_sample_rate_changed(Vst::SampleRate in_sample_rate) {
	current_sample_rate = in_sample_rate;
	update_phase_delta(sin_freq_parameter.getValue(), in_sample_rate);
}

void sin_tone_processor::update_phase_delta(Vst::Sample64 in_freq, Vst::SampleRate in_sample_rate) {
	delta_phase = T_PI * in_freq / in_sample_rate;
}

Vst::Sample64 sin_tone_processor::get_sine() {
	const Vst::Sample64 tone = std::sin(current_phase);
	current_phase += delta_phase;

	// invert phase
	if (current_phase >= T_PI)
		current_phase -= T_PI;
	return tone;
}

FUnknown *create_processor_instance(void *in_ptr) {
	return static_cast<Steinberg::Vst::IAudioProcessor *>(new sin_tone_processor);
}

} // namespace Steinberg::Tutorial
