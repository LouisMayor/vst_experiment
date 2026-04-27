//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

// VSTGUI support
#include "vstgui/plugin-bindings/vst3editor.h"

namespace VstguiHelloWorldVendor {

//------------------------------------------------------------------------
//  VstguiHelloWorldController
//------------------------------------------------------------------------
class VstguiHelloWorldController : public Steinberg::Vst::EditControllerEx1, public VSTGUI::VST3EditorDelegate {
  public:
	using UTF8StringPtr	 = VSTGUI::UTF8StringPtr;
	using IUIDescription = VSTGUI::IUIDescription;
	using IController	 = VSTGUI::IController;
	using VST3Editor	 = VSTGUI::VST3Editor;

	//------------------------------------------------------------------------
	VstguiHelloWorldController()				= default;
	~VstguiHelloWorldController() SMTG_OVERRIDE = default;

	// Create function
	static Steinberg::FUnknown *createInstance(void * /*context*/) {
		return (Steinberg::Vst::IEditController *)new VstguiHelloWorldController;
	}

	//--- from IPluginBase -----------------------------------------------
	Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown *context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	//--- from EditController --------------------------------------------
	Steinberg::tresult PLUGIN_API	 setComponentState(Steinberg::IBStream *state) SMTG_OVERRIDE;
	Steinberg::IPlugView *PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API	 setState(Steinberg::IBStream *state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API	 getState(Steinberg::IBStream *state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API	 setParamNormalized(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;

	//--- from VST3EditorDelegate ----------------------------------------
	IController *createSubController(UTF8StringPtr name, const IUIDescription *description, VST3Editor *editor) SMTG_OVERRIDE;

	//---Interface---------
	DEFINE_INTERFACES
	// Here you can add more supported VST3 interfaces
	// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES(EditController)
	DELEGATE_REFCOUNT(EditController)

	//------------------------------------------------------------------------
  protected:
};

//------------------------------------------------------------------------
} // namespace VstguiHelloWorldVendor
