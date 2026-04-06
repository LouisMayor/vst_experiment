# Get VST3 SDK and update CMAKE to include the SDK
## DESCRIPTION
  - Download/clone the VST3 SDK from https://github.com/steinbergmedia/vst3sdk
  - Update CMakeLists.txt to include the SDK properly
  - Use `https://github.com/steinbergmedia/vst3_tutorials/tree/33b73dfbb87f3fde3bce8c0a10cae934dc66ad34/advanced-techniques-tutorial` as reference for CMake integration
  - Ensure cross-platform support (Windows, MacOS, Linux)
## RELEVANT INFOMATION
  - requirements file: project_management/requirements/startup_requirements.md (lines 4-6)
  - current CMakeLists.txt: CMakeLists.txt (needs modification)
  - thirdparty location: src/thirdparty/ (per CLAUDE.md convention)
  - considerations: 
    - Follow C++23 best practices
    - Use snake_case naming
    - Keep CMake 4.3 requirement
## CHANGES LIST
  - Cloned VST3 SDK from https://github.com/steinbergmedia/vst3sdk to src/thirdparty/vst3sdk
  - Initialized git submodules (base, cmake, doc, pluginterfaces, public.sdk, tutorials, vstgui4)
  - Updated CMakeLists.txt to include VST3 SDK via add_subdirectory
  - Added SMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF, SMTG_ENABLE_VST3_HOSTING_EXAMPLES=OFF, SMTG_ENABLE_VSTGUI_SUPPORT=OFF to disable unnecessary builds
  - Build verified successfully on macOS with Xcode
