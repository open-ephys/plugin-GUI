#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
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
	)
	
#optional: create IDE groups
plugin_create_filters()
