if(TARGET vst_common)
	return()
endif()

add_library(vst_common INTERFACE)

target_include_directories(vst_common
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vst_common
	INTERFACE
		sdk
)
