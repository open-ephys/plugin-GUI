#Open Ephys GUI main plugin build file. Cannot be used standalone, but called by the main GUI build file
cmake_minimum_required(VERSION 3.5.0)

project(open-ephys-plugins)
set(PLUGIN_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Headers)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
			OEPLUGIN
			"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
			)
				
#add plugin subdirectories
add_subdirectory(CAR)
add_subdirectory(BinaryFormat)