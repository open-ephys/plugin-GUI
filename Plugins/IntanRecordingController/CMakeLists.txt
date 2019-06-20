#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	rhythm-api/okFrontPanelDLL.h
	rhythm-api/rhd2000datablockusb3.cpp
	rhythm-api/rhd2000datablockusb3.h
	rhythm-api/rhd2000evalboardusb3.cpp
	rhythm-api/rhd2000evalboardusb3.h
	rhythm-api/rhd2000registersusb3.cpp
	rhythm-api/rhd2000registersusb3.h
	RHD2000Thread.cpp
	RHD2000Thread.h
	RHD2000Editor.cpp
	RHD2000Editor.h
	USBThread.cpp
	USBThread.h
	)

if (MSVC)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/okFrontPanel.lib")
elseif(LINUX)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/libokFrontPanel.so")
elseif(APPLE)
	target_link_libraries(${PLUGIN_NAME} "${DLLDIR}/libokFrontPanel.dylib")
	set_source_files_properties(${DLLDIR}/libokFrontPanel.dylib PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
	target_sources(${PLUGIN_NAME} PRIVATE ${DLLDIR}/libokFrontPanel.dylib)
endif()

#optional: create IDE groups
plugin_create_filters()
