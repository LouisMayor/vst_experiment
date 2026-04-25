if(TARGET vst_common)
	return()
endif()

add_library(vst_common STATIC
	source/frequency_parameter.cpp
)

target_include_directories(vst_common
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vst_common
	PUBLIC
		sdk
)
