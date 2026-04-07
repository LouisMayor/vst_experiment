smtg_add_vst3plugin(gain-vst3
    source/cids.h
    source/controller.cpp
    source/entry.cpp
    source/pids.h
    source/processor.cpp
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