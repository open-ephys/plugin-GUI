cmake_minimum_required(VERSION 3.15)

add_library(test_helpers SHARED)

target_compile_features(test_helpers PRIVATE cxx_std_17)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
			OEPLUGIN
			"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
			)

add_dependencies(test_helpers gui_testable_source)
target_include_directories(test_helpers
        PRIVATE
        ${JUCE_DIRECTORY}
        ${JUCE_DIRECTORY}/modules
        ${PLUGINS_DIRECTORY}/Headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PUBLIC
        ${SOURCE_DIRECTORY}
        )
target_link_libraries(test_helpers PRIVATE gui_testable_source)

set_property(TARGET test_helpers PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BIN_TESTS_DIR}/common)
set_property(TARGET test_helpers PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BIN_TESTS_DIR}/common)

add_subdirectory(Processors)
add_subdirectory(Application)
