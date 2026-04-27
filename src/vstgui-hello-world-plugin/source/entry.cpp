//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#include "cids.h"
#include "controller.h"
#include "processor.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "vstgui-hello-world-plugin"

using namespace Steinberg::Vst;
using namespace VstguiHelloWorldVendor;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF("My Plug-in Company", "https://www.mycompanyname.com", "mailto:info@mycompanyname.com")

//---First Plug-in included in this factory-------
// its kVstAudioEffectClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kVstguiHelloWorldProcessorUID),
		   PClassInfo::kManyInstances,				  // cardinality
		   kVstAudioEffectClass,					  // the component category (do not changed this)
		   stringPluginName,						  // here the Plug-in name (to be changed)
		   Vst::kDistributable,						  // means that component and controller could be distributed on different computers
		   VstguiHelloWorldVST3Category,			  // Subcategory for this Plug-in (to be changed)
		   FULL_VERSION_STR,						  // Plug-in version (to be changed)
		   kVstVersionString,						  // the VST 3 SDK version (do not changed this, use always this define)
		   VstguiHelloWorldProcessor::createInstance) // function pointer called when this component should be instantiated

// its kVstComponentControllerClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kVstguiHelloWorldControllerUID),
		   PClassInfo::kManyInstances,				   // cardinality
		   kVstComponentControllerClass,			   // the Controller category (do not changed this)
		   stringPluginName "Controller",			   // controller name (could be the same than component name)
		   0,										   // not used here
		   "",										   // not used here
		   FULL_VERSION_STR,						   // Plug-in version (to be changed)
		   kVstVersionString,						   // the VST 3 SDK version (do not changed this, use always this define)
		   VstguiHelloWorldController::createInstance) // function pointer called when this component should be instantiated

//----for others Plug-ins contained in this factory, put like for the first Plug-in different DEF_CLASS2---

END_FACTORY
