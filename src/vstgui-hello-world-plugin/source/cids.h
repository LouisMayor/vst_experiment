//------------------------------------------------------------------------
// Copyright(c) 2026 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace VstguiHelloWorldVendor {
//------------------------------------------------------------------------
static const Steinberg::FUID kVstguiHelloWorldProcessorUID(0xC88ECEF1, 0x19855631, 0x8F13F23B, 0x4F946EDC);
static const Steinberg::FUID kVstguiHelloWorldControllerUID(0x1923B0EC, 0xB8885A7D, 0x9F689C3D, 0x175F7A08);

#define VstguiHelloWorldVST3Category "Fx"

// Parameter IDs
enum {
	kGainId = 0, ///< Gain parameter (0..1 normalized)
};

//------------------------------------------------------------------------
} // namespace VstguiHelloWorldVendor
