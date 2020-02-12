#plugin build file
cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 17)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	BinaryRecording.cpp
	BinaryRecording.h
	DataQueue.cpp
	DataQueue.h
	EventQueue.h
	FileMemoryBlock.h
	NpyFile.h
	NpyFile.cpp
	OriginalRecording.h
	OriginalRecording.cpp
	RecordChannelSelector.h
	RecordChannelSelector.cpp	
	RecordEngine.h
	RecordEngine.cpp
	RecordNode.h 
	RecordNode.cpp
	RecordNodeEditor.h
	RecordNodeEditor.cpp 
	RecordThread.h
	RecordThread.cpp 
	SequentialBlockFile.h
	SequentialBlockFile.cpp 
	Utils.h
)
#optional: create IDE groups
#plugin_create_filters()