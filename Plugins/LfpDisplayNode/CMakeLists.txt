#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources
add_sources(${PLUGIN_NAME}
	OpenEphysLib.cpp
	LfpDisplayNode.cpp
	LfpDisplayNode.h
	LfpDisplayEditor.cpp
	LfpDisplayEditor.h
	DisplayBuffer.cpp
	DisplayBuffer.h
	EventDisplayInterface.cpp
	EventDisplayInterface.h
	LfpBitmapPlotter.h
	LfpBitmapPlotterInfo.h
	LfpChannelDisplay.cpp
	LfpChannelDisplay.h
	LfpChannelDisplayInfo.cpp
	LfpChannelDisplayInfo.h
	LfpDisplay.cpp
	LfpDisplay.h
	LfpDisplayClasses.h
	LfpDisplayCanvas.cpp
	LfpDisplayCanvas.h
	LfpDisplayOptions.cpp
	LfpDisplayOptions.h
	LfpTimescale.cpp
	LfpTimescale.h
	LfpViewport.cpp
	LfpViewport.h
	PerPixelBitmapPlotter.cpp
	PerPixelBitmapPlotter.h
	ShowHideOptionsButton.cpp
	ShowHideOptionsButton.h
	SupersampledBitmapPlotter.cpp
	SupersampledBitmapPlotter.h
	ColourSchemes/ChannelColourScheme.cpp
	ColourSchemes/ChannelColourScheme.h
	ColourSchemes/DefaultColourScheme.cpp
	ColourSchemes/DefaultColourScheme.h
	ColourSchemes/MonochromeGrayColourScheme.cpp
	ColourSchemes/MonochromeGrayColourScheme.h
	ColourSchemes/MonochromeYellowColourScheme.cpp
	ColourSchemes/MonochromeYellowColourScheme.h
	ColourSchemes/MonochromeGreenColourScheme.cpp
	ColourSchemes/MonochromeGreenColourScheme.h
	ColourSchemes/MonochromePurpleColourScheme.cpp
	ColourSchemes/MonochromePurpleColourScheme.h
	ColourSchemes/OELogoColourScheme.cpp
	ColourSchemes/OELogoColourScheme.h
	ColourSchemes/TropicalColourScheme.cpp
	ColourSchemes/TropicalColourScheme.h
	ColourSchemes/LightBackgroundColourScheme.cpp
	ColourSchemes/LightBackgroundColourScheme.h
	)

if (CMAKE_CONFIGURATION_TYPES)
	# Multi-configuration
	set(GUI_TEST_BIN_COMMON_DIR ${GUI_BASE_DIR}/Build/TestBin/common)
else()
	set(GUI_TEST_BIN_COMMON_DIR ${GUI_BASE_DIR}/Build/${CMAKE_BUILD_TYPE}/TestBin/common)
endif()
set(GUI_TEST_HELPERS_DIR ${GUI_BASE_DIR}/TestHelpers)

if(BUILD_TESTS)
    copy_sources(${PLUGIN_NAME} ${PLUGIN_NAME}_testable)
    add_subdirectory(Tests)
	target_include_directories(${PLUGIN_NAME}_tests PRIVATE ${GUI_TEST_HELPERS_DIR}/include)

	if(MSVC)
		add_custom_command(TARGET ${PLUGIN_NAME}_tests POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gui_testable_source> $<TARGET_FILE_DIR:${PLUGIN_NAME}_tests>)
		add_custom_command(TARGET ${PLUGIN_NAME}_tests POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test_helpers> $<TARGET_FILE_DIR:${PLUGIN_NAME}_tests>)	
	endif()
endif()
	
#optional: create IDE groups
plugin_create_filters()
