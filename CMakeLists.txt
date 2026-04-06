cmake_minimum_required(VERSION 4.3)
project(vst LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(RELEASE=1)

if(CMAKE_VERSION VERSION_LESS "3.21")
    set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES_CONCAT "")
endif()

set(vst3sdk_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/thirdparty/vst3sdk" CACHE PATH "Path to VST3 SDK source" FORCE)

if(NOT EXISTS "${vst3sdk_SOURCE_DIR}")
    message(FATAL_ERROR "VST3 SDK not found at ${vst3sdk_SOURCE_DIR}. Please clone https://github.com/steinbergmedia/vst3sdk")
endif()

set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES 0)
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES 0)
set(SMTG_ENABLE_VSTGUI_SUPPORT 0)

add_subdirectory(${vst3sdk_SOURCE_DIR} ${CMAKE_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

set(SMTG_RUN_VST_VALIDATOR OFF)

smtg_add_vst3plugin(gain-vst3
    src/plugin/source/cids.h
    src/plugin/source/controller.cpp
    src/plugin/source/entry.cpp
    src/plugin/source/pids.h
    src/plugin/source/processor.cpp
)

target_compile_features(gain-vst3 PRIVATE cxx_std_23)

target_link_libraries(gain-vst3 PRIVATE sdk)

smtg_target_configure_version_file(gain-vst3)

if(APPLE)
    smtg_target_set_bundle(gain-vst3
        BUNDLE_IDENTIFIER com.louismayor.vst3.gain
        COMPANY_NAME "Louismayor"
    )
    smtg_target_set_debug_executable(gain-vst3
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
endif()
