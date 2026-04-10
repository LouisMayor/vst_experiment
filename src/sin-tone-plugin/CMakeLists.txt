smtg_add_vst3plugin(sin_tone-vst3
    source/cids.h
    source/controller.cpp
    source/entry.cpp
    source/pids.h
    source/processor.cpp
)

target_compile_features(sin_tone-vst3 PRIVATE cxx_std_23)

target_link_libraries(sin_tone-vst3 PRIVATE sdk)

smtg_target_configure_version_file(sin_tone-vst3)

if(APPLE)
    smtg_target_set_bundle(sin_tone-vst3
        BUNDLE_IDENTIFIER com.louismayor.vst3.sin_tone
        COMPANY_NAME "Louismayor"
    )
    smtg_target_set_debug_executable(sin_tone-vst3
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
endif()
