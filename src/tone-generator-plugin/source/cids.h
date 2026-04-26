//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace ToneGeneratorVendor {
//------------------------------------------------------------------------
static const Steinberg::FUID kToneGeneratorProcessorUID (0x8759EB4E, 0xB0B05137, 0x92D22F9E, 0xDD24EF4C);
static const Steinberg::FUID kToneGeneratorControllerUID (0x5CC59E58, 0x96D25BCE, 0x91A0847A, 0xE43F7133);

#define ToneGeneratorVST3Category "Instrument"

//------------------------------------------------------------------------
} // namespace ToneGeneratorVendor
