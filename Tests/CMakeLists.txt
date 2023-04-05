cmake_minimum_required(VERSION 3.5.0)

project(open-ephys-components)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
			OEPLUGIN
			"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
			)


add_subdirectory(Processors)
