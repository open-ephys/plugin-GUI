#Open Ephys GUI main plugin build file. Cannot be used standalone, but called by the main GUI build file
cmake_minimum_required(VERSION 3.5.0)

project(open-ephys-plugins)
set(PLUGIN_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Headers)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
			OEPLUGIN
			"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
			)
				
#add plugin subdirectories
add_subdirectory(ArduinoOutput)
add_subdirectory(BasicSpikeDisplay)
add_subdirectory(CAR)
add_subdirectory(ChannelMappingNode)
add_subdirectory(EvntTrigAvg)
add_subdirectory(FilterNode)
add_subdirectory(IntanRecordingController)
add_subdirectory(LfpDisplayNode)
add_subdirectory(LfpDisplayNodeBeta)
add_subdirectory(PhaseDetector)
add_subdirectory(PulsePalOutput)
add_subdirectory(RecordControl)
add_subdirectory(Rectifier)
add_subdirectory(RhythmNode)
add_subdirectory(SerialInput)
add_subdirectory(SpikeSorter)