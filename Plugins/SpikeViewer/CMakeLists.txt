#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	SpikeDisplayCanvas.cpp
	SpikeDisplayCanvas.h
	SpikeDisplay.cpp
	SpikeDisplay.h
	SpikePlots.cpp
	SpikePlots.h
	SpikeDisplayEditor.cpp
	SpikeDisplayEditor.h
	SpikeDisplayNode.cpp
	SpikeDisplayNode.h
	)

if(BUILD_TESTS)
	copy_sources(${PLUGIN_NAME} ${PLUGIN_NAME}_testable)
    add_subdirectory(Tests)
endif()