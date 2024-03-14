#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	serial/ofArduino.cpp
	serial/ofArduino.h
	ArduinoOutput.cpp
	ArduinoOutput.h
	ArduinoOutputEditor.cpp
	ArduinoOutputEditor.h
	)
if(MSVC)
	target_link_libraries(${PLUGIN_NAME} Winmm.lib)
endif()

if(BUILD_TESTS)
		if(MSVC)
			target_link_libraries(${PLUGIN_NAME}_testable PUBLIC Winmm.lib)
		endif()
    	copy_sources(${PLUGIN_NAME} ${PLUGIN_NAME}_testable)
     	add_subdirectory(Tests)
endif()
#optional: create IDE groups
plugin_create_filters()
