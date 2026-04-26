#pragma once

#include "public.sdk/source/vst/utility/rttransfer.h"
#include "public.sdk/source/vst/utility/sampleaccurate.h"

#include <memory>

namespace VstCommon {

/**
 * DSP adapter: wraps Steinberg::Vst::SampleAccurate::Parameter for a given ParameterModel.
 *
 * Provides compile-time access to the Model's conversion logic via getPlainValue().
 * Thread-safe state restoration is built-in: setStateValue() queues a value from the
 * non-realtime thread, and applyPendingTransfer() applies it on the realtime thread.
 *
 * Usage:
 *   VstCommon::ModelSampleAccurateParameter<MyModel> param;
 *   // in setState (non-RT):
 *   param.setStateValue(newValue);
 *   // in process (RT):
 *   param.applyPendingTransfer();
 *   auto plain = param.advancePlain(numSamples);  // advance + convert
 *   // or
 *   param.advance(numSamples);
 *   auto plain = param.getPlainValue();           // convert current value
 */
template <typename Model> class ModelSampleAccurateParameter {
  public:
	ModelSampleAccurateParameter() : param(Model::kId, Model::toNormalized(Model::kDefaultPlain)) {}

	/**
	 * Queue a value to be applied safely from the realtime thread.
	 *
	 * Call this from non-RT contexts (e.g. setState).
	 */
	void setStateValue(Steinberg::Vst::ParamValue v) {
		auto state	 = std::make_unique<ParamValueState>();
		state->value = v;
		transfer.transferObject_ui(std::move(state));
	}

	/**
	 * Apply any pending value transferred via setStateValue.
	 *
	 * Call this once per process() call, on the realtime thread,
	 * before using advance() / beginChanges().
	 */
	void applyPendingTransfer() {
		transfer.accessTransferObject_rt([this](const auto &state) { param.setValue(state.value); });
	}

	/**
	 * Convert the current normalized value to a plain value using Model::toPlain.
	 */
	Steinberg::Vst::ParamValue getPlainValue() const {
		return Model::toPlain(getValue());
	}

	/**
	 * Advance the parameter by numSamples and return the new plain value.
	 */
	Steinberg::Vst::ParamValue advancePlain(Steinberg::int32 numSamples) {
		advance(numSamples);
		return getPlainValue();
	}

	// -------------------------------------------------------------------
	// Forwarding to the underlying SampleAccurate::Parameter
	// -------------------------------------------------------------------

	void setValue(Steinberg::Vst::ParamValue v) noexcept {
		param.setValue(v);
	}

	void setParamID(Steinberg::Vst::ParamID pid) noexcept {
		param.setParamID(pid);
	}

	Steinberg::Vst::ParamID getParamID() const noexcept {
		return param.getParamID();
	}

	Steinberg::Vst::ParamValue getValue() const noexcept {
		return param.getValue();
	}

	bool hasChanges() const noexcept {
		return param.hasChanges();
	}

	void beginChanges(Steinberg::Vst::IParamValueQueue *valueQueue) noexcept {
		param.beginChanges(valueQueue);
	}

	Steinberg::Vst::ParamValue advance(Steinberg::int32 numSamples) noexcept {
		return param.advance(numSamples);
	}

	Steinberg::Vst::ParamValue flushChanges() noexcept {
		return param.flushChanges();
	}

	Steinberg::Vst::ParamValue endChanges() noexcept {
		return param.endChanges();
	}

	template <typename Proc> void advance(Steinberg::int32 numSamples, Proc p) {
		param.advance(numSamples, p);
	}

	template <typename Proc> void flushChanges(Proc p) {
		param.flushChanges(p);
	}

	template <typename Proc> void endChanges(Proc p) {
		param.endChanges(p);
	}

  private:
	struct ParamValueState {
		Steinberg::Vst::ParamValue value = 0.0;
	};

	Steinberg::Vst::SampleAccurate::Parameter	 param;
	Steinberg::Vst::RTTransferT<ParamValueState> transfer;
};

} // namespace VstCommon
