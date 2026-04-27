cmake_minimum_required(VERSION 3.14.0)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../thirdparty/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(vstgui-hello-world-vst3
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "vstgui-hello-world-vst3 VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(vstgui-hello-world-vst3
    source/version.h
    source/cids.h
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(vstgui-hello-world-vst3
        PRIVATE
            resource/editor.uidesc
    )
    target_link_libraries(vstgui-hello-world-vst3
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(vstgui-hello-world-vst3
        RESOURCES
            "resource/editor.uidesc"
    )
    # Enable VSTGUI live UI editing in Debug builds
    target_compile_definitions(vstgui-hello-world-vst3
        PRIVATE
            $<$<CONFIG:Debug>:VSTGUI_LIVE_EDITING=1>
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (vstgui-hello-world-vst3
    RESOURCES
        resource/C88ECEF1198556318F13F23B4F946EDC_snapshot.png
        resource/C88ECEF1198556318F13F23B4F946EDC_snapshot_2.0x.png
)

target_link_libraries(vstgui-hello-world-vst3
    PRIVATE
        sdk
)

smtg_target_configure_version_file(vstgui-hello-world-vst3)

if(SMTG_MAC)
    smtg_target_set_bundle(vstgui-hello-world-vst3
        BUNDLE_IDENTIFIER com.example.vst3.vstgui_hello_world
        COMPANY_NAME "My Plug-in Company"
    )
    smtg_target_set_debug_executable(vstgui-hello-world-vst3
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(vstgui-hello-world-vst3 PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vstgui-hello-world-vst3)

        smtg_target_set_debug_executable(vstgui-hello-world-vst3
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
