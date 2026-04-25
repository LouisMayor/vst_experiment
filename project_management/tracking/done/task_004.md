# Create volume parameter
## DESCRIPTION
  - Create a volume parameter following the same pattern as the existing frequency parameter
  - Use ModelRangeParameter adapter with a VolumeModel struct
  - Range: -60 dB to 0 dB with default at -6 dB
  - Ensure consistent include style and naming conventions

## RELEVANT INFOMATION
  - Reference implementation: frequency_model.h and frequency_parameter.h
  - Uses Steinberg VST3 SDK types and STR16 macro for string literals
  - Part of vst_common library (header-only interface library)

## CHANGES LIST
  - Created src/common/include/vst_common/volume_model.h with VolumeModel struct
    - ParamID: 101, Title: "Volume", Units: "dB"
    - Linear mapping in dB space (-60 to 0 dB)
    - toPlain, toNormalized, toString, fromString methods
  - Created src/common/include/vst_common/volume_parameter.h with VolumeParameter alias
  - Fixed include style to use quotes (consistent with frequency_parameter.h)
  - Verified compilation by building tone-generator-plugin successfully
