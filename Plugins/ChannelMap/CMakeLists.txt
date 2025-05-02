# plugin build file
cmake_minimum_required(VERSION 3.15)

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

if(APPLE)
  set_target_properties(
    ${PLUGIN_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                              "org.open-ephys.plugin.ChannelMap")
endif()

if(BUILD_TESTS)
  add_subdirectory(Tests)
endif()
