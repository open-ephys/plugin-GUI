#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	Source/NeuropixEditor.cpp
	Source/NeuropixEditor.h
	Source/NeuropixThread.cpp
	Source/NeuropixThread.h
	#Source/neuropix-api/AdcPacket.h
	#Source/neuropix-api/BaseConfiguration.h
	#Source/neuropix-api/dll_import_export.h
	#Source/neuropix-api/ElectrodePacket.h
	#Source/neuropix-api/hs_fpga_register_def.h
	#Source/neuropix-api/Neuropix_basestation_api.h
	#Source/neuropix-api/ShankConfiguration.h
	#Source/neuropix-api/TestConfiguration.h
	)
if (MSVC)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/libNeuropix_basestation_api_msvc.lib")
endif()

set(NEUROPIX_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/neuropix-api)
target_include_directories(${PLUGIN_NAME} PRIVATE ${NEUROPIX_INC_DIR})

#optional: create IDE groups
plugin_create_filters()