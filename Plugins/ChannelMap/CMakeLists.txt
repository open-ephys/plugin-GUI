# plugin build file
cmake_minimum_required(VERSION 3.5.0)

# include common rules
include(../PluginRules.cmake)

# add sources, not including OpenEphysLib.cpp
add_sources(
  ${PLUGIN_NAME}
  ChannelMapActions.cpp
  ChannelMapActions.h
  ChannelMap.cpp
  ChannelMap.h
  ChannelMapEditor.cpp
  ChannelMapEditor.h
  PrbFormat.h)

if(BUILD_TESTS)
  add_subdirectory(Tests)
endif()
