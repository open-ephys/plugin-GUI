# plugin build file
cmake_minimum_required(VERSION 3.15)

# include common rules
include(../PluginRules.cmake)

# add sources, not including OpenEphysLib.cpp
add_sources(
  ${PLUGIN_NAME}
  SpikeDisplayCanvas.cpp
  SpikeDisplayCanvas.h
  SpikeDisplay.cpp
  SpikeDisplay.h
  SpikePlots.cpp
  SpikePlots.h
  SpikeDisplayEditor.cpp
  SpikeDisplayEditor.h
  SpikeDisplayNode.cpp
  SpikeDisplayNode.h)

if(APPLE)
  set_target_properties(
    ${PLUGIN_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                              "org.open-ephys.plugin.SpikeViewer")
endif()

if(BUILD_TESTS)
  add_subdirectory(Tests)
endif()
