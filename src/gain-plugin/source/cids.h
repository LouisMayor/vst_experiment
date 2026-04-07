#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace Steinberg::Tutorial {
static const FUID processor_uid (0xC18D3C1E, 0x719E4E29, 0x924D3ECA, 0xA5E4DA18);
static const FUID controller_uid (0xC244B7E6, 0x24084E20, 0x24A8C43, 0xF84C8BE8);

#define gain_vst3_category "Fx"

}
