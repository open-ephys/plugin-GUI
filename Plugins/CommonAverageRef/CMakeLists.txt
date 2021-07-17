#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	CommonAverageRef.cpp
	CommonAverageRef.h
	CommonAverageRefEditor.cpp
	CommonAverageRefEditor.h
	)
	
#optional: create IDE groups
#plugin_create_filters()