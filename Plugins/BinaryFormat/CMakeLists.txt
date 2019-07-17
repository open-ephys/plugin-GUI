#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	FileSource/BinaryFileSource.cpp
	FileSource/BinaryFileSource.h
	RecordEngine/BinaryRecording.cpp
	RecordEngine/BinaryRecording.h
	RecordEngine/FileMemoryBlock.h
	RecordEngine/NpyFile.cpp
	RecordEngine/NpyFile.h
	RecordEngine/SequentialBlockFile.cpp
	RecordEngine/SequentialBlockFile.h
	)
	
#optional: create IDE groups
plugin_create_filters()