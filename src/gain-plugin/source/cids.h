//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace GainVendor {
//------------------------------------------------------------------------
static const Steinberg::FUID kGainProcessorUID (0x50E31F29, 0x62FA5BDB, 0x9BE08EFD, 0x501EE34B);
static const Steinberg::FUID kGainControllerUID (0xB2ECB30D, 0xCDF2582B, 0xA4074C21, 0xFD5BB835);

#define GainVST3Category "Fx"

//------------------------------------------------------------------------
} // namespace GainVendor
