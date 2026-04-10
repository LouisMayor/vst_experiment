#include "cids.h"
#include "public.sdk/source/main/pluginfactory.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#define string_plugin_name "sin_tone-vst3"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace Steinberg::Tutorial;

namespace Steinberg::Tutorial {
FUnknown* create_processor_instance(void*);
FUnknown* create_controller_instance(void*);
}

BEGIN_FACTORY_DEF("Louismayor",
		       "https://github.com/louismayor",
		       "mailto:louismayor@example.com")

	DEF_CLASS2(INLINE_UID_FROM_FUID(processor_uid),
			PClassInfo::kManyInstances,
			kVstAudioEffectClass,
			string_plugin_name,
			Vst::kDistributable,
			sin_tone_vst3_category,
			"1.0.0",
			kVstVersionString,
			create_processor_instance)

	DEF_CLASS2(INLINE_UID_FROM_FUID(controller_uid),
			PClassInfo::kManyInstances,
			kVstComponentControllerClass,
			string_plugin_name "Controller",
			0,
			"",
			"1.0.0",
			kVstVersionString,
			create_controller_instance)

END_FACTORY
