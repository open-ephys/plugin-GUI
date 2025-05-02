# plugin build file
cmake_minimum_required(VERSION 3.15)

# include common rules
include(../PluginRules.cmake)

# add sources, not including OpenEphysLib.cpp
add_sources(
  ${PLUGIN_NAME}
  serial/ofArduino.cpp
  serial/ofArduino.h
  ArduinoOutput.cpp
  ArduinoOutput.h
  ArduinoOutputEditor.cpp
  ArduinoOutputEditor.h)
if(MSVC)
  target_link_libraries(${PLUGIN_NAME} Winmm.lib)
elseif(APPLE)
  set_target_properties(
    ${PLUGIN_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                              "org.open-ephys.plugin.ArduinoOutput")
endif()

if(BUILD_TESTS)
  add_subdirectory(Tests)
endif()
