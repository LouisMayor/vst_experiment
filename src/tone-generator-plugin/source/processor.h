//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/vst/vsttypes.h"
#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "vst_common/frequency_model.h"
#include "vst_common/model_sample_accurate_parameter.h"
#include "vst_common/volume_model.h"
#include "vst_common/waveform_parameter.h"

namespace ToneGeneratorVendor {

//------------------------------------------------------------------------
//  ToneGeneratorProcessor
//------------------------------------------------------------------------
class ToneGeneratorProcessor : public Steinberg::Vst::AudioEffect {
  public:
	ToneGeneratorProcessor();
	~ToneGeneratorProcessor() SMTG_OVERRIDE;

	// Create function
	static Steinberg::FUnknown *createInstance(void * /*context*/) {
		return (Steinberg::Vst::IAudioProcessor *)new ToneGeneratorProcessor;
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context) SMTG_OVERRIDE;

	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup &newSetup) SMTG_OVERRIDE;

	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData &data) SMTG_OVERRIDE;

	/** For persistence */
	Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state) SMTG_OVERRIDE;

	//------------------------------------------------------------------------
  protected:
	void handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes);

	// todo: move to src/common/include/vst_common/model_sample_accurate_parameter.h
	struct RTTransfer_FreqModel {
		Steinberg::Vst::Sample64 freq;
	};
	using freq_rt_transfer = Steinberg::Vst::RTTransferT<RTTransfer_FreqModel>;

	freq_rt_transfer												   freq_rt;
	VstCommon::ModelSampleAccurateParameter<VstCommon::FrequencyModel> freq_param;
	VstCommon::ModelSampleAccurateParameter<VstCommon::VolumeModel>	   vol_param;
	VstCommon::WaveformSampleAccurateParameter						   waveform_param;

	Steinberg::Vst::Sample64 phase = 0.0;
};

//------------------------------------------------------------------------
} // namespace ToneGeneratorVendor
