//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include "pid.h"

namespace GainVendor {

//------------------------------------------------------------------------
//  GainProcessor
//------------------------------------------------------------------------
class GainProcessor : public Steinberg::Vst::AudioEffect {
  public:
	GainProcessor();
	~GainProcessor() SMTG_OVERRIDE;

	// Create function
	static Steinberg::FUnknown *createInstance(void * /*context*/) {
		return (Steinberg::Vst::IAudioProcessor *)new GainProcessor;
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

	void handle_parameter_changes(Steinberg::Vst::IParameterChanges *in_changes);

  protected:
	struct gain_param {
		Steinberg::Vst::Sample64 gain_level;
	};
	using gain_rt_transfer = Steinberg::Vst::RTTransferT<gain_param>;
	gain_rt_transfer gain_rt;

	Steinberg::Vst::SampleAccurate::Parameter gain_parameter{parameter_id::gain, 1.};
};

//------------------------------------------------------------------------
} // namespace GainVendor
