cmake_minimum_required(VERSION 3.5.0)

project(open-ephys-components)
set(PLUGIN_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Plugins/Headers)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
			OEPLUGIN
			"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
			)

add_subdirectory(Processors)
add_subdirectory(UI)