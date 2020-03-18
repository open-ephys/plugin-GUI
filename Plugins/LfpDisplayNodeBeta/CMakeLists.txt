#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
        LfpChannelDisplay.cpp
        LfpChannelDisplay.h
        LfpDisplayCanvas.cpp
        LfpDisplayCanvasElements.cpp
        LfpDisplayCanvasElements.h
        LfpDisplayCanvas.h
        LfpDisplay.cpp
        LfpDisplayEditor.cpp
        LfpDisplayEditor.h
        LfpDisplay.h
        LfpDisplayNode.cpp
        LfpDisplayNode.h
        LfpDisplayOptions.cpp
        LfpDisplayOptions.h
	EventDisplayInterface.cpp
	EventDisplayInterface.h
        OpenEphysLib.cpp
        )
	
#optional: create IDE groups
#plugin_create_filters()
