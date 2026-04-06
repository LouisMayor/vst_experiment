# Create VST3 Plugin (from advanced technique tutorial)
## DESCRIPTION
  - Create a VST3 audio effect plugin following the advanced techniques tutorial
  - Implement sample accurate parameter handling using ProcessDataSlicer and SampleAccurate::Parameter
  - Add 32 and 64 bit audio processing using C++ templates
  - Implement thread safe state changes using RTTransferT
  - Reference: https://github.com/steinbergmedia/vst3_tutorials/tree/main/advanced-techniques-tutorial
  - The plugin should be a gain effect that demonstrates all three advanced techniques
## RELEVANT INFOMATION
  - requirements file: project_management/requirements/startup_requirements.md (line 7-8)
  - depends on task_002: VST3 SDK must be integrated first
  - source location: src/
  - SDK utility classes needed:
    - public.sdk/source/vst/utility/processdataslicer.h
    - public.sdk/source/vst/utility/sampleaccurate.h
    - public.sdk/source/vst/utility/rttransfer.h
  - considerations:
    - Follow C++23 best practices
    - Use snake_case naming convention
    - Use smart pointers (no dumb pointers)
    - Support cross-platform (Windows, MacOS, Linux)
## CHANGES LIST
  - Created src/plugin/source/ directory with plugin implementation files
  - Created cids.h with processor and controller UIDs
  - Created pids.h with parameter ID definitions (gain parameter)
  - Created processor.cpp implementing:
    - SampleAccurate::Parameter for gain control
    - ProcessDataSlicer for processing audio in chunks
    - RTTransferT for thread-safe state transfer between UI and RT threads
    - Template-based 32/64-bit audio processing support
  - Created controller.cpp with gain parameter definition
  - Created entry.cpp with plugin factory registration
  - Updated CMakeLists.txt to build the VST3 plugin using smtg_add_vst3plugin
  - Plugin built successfully at build/VST3/gain-vst3.vst3/
    - Note: SDK now uses C++23 with a fix in threadchecker_mac.mm (added #include for std::terminate)
