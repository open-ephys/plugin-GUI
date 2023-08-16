#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	ChannelMappingNode.cpp
	ChannelMappingNode.h
	ChannelMappingEditor.cpp
	ChannelMappingEditor.h
	PrbFormat.h
	)

if(BUILD_TESTS)
    copy_sources(${PLUGIN_NAME} ${PLUGIN_NAME}_testable)
    add_subdirectory(Tests)
endif()
	
#optional: create IDE groups
#plugin_create_filters()
