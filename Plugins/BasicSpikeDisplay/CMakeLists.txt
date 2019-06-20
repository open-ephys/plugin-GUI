#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	SpikeDetector/SpikeDetector.cpp
	SpikeDetector/SpikeDetector.h
	SpikeDetector/SpikeDetectorEditor.cpp
	SpikeDetector/SpikeDetectorEditor.h
	SpikeDisplayNode/SpikeDisplayCanvas.cpp
	SpikeDisplayNode/SpikeDisplayCanvas.h
	SpikeDisplayNode/SpikeDisplayEditor.cpp
	SpikeDisplayNode/SpikeDisplayEditor.h
	SpikeDisplayNode/SpikeDisplayNode.cpp
	SpikeDisplayNode/SpikeDisplayNode.h
	)
	
#optional: create IDE groups
plugin_create_filters()