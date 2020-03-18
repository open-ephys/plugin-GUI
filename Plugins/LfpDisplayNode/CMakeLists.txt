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
	EventDisplayInterface.cpp
	EventDisplayInterface.h
	LfpBitmapPlotter.h
	LfpBitmapPlotterInfo.h
	LfpChannelColourScheme.cpp
	LfpChannelColourScheme.h
	LfpChannelDisplay.cpp
	LfpChannelDisplay.h
	LfpChannelDisplayInfo.cpp
	LfpChannelDisplayInfo.h
	LfpDefaultColourScheme.cpp
	LfpDefaultColourScheme.h
	LfpDisplay.cpp
	LfpDisplay.h
	LfpDisplayCanvas.cpp
	LfpDisplayCanvas.h
	LfpDisplayOptions.cpp
	LfpDisplayOptions.h
	LfpGradientColourScheme.cpp
	LfpGradientColourScheme.h
	LfpMonochromaticColourScheme.cpp
	LfpMonochromaticColourScheme.h
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
	)
	
#optional: create IDE groups
#plugin_create_filters()
