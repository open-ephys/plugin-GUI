#Open Ephys GUI main build file
cmake_minimum_required(VERSION 3.5.0)

set(GUI_VERSION 0.4.5)

string(REGEX MATCHALL "[0-9]+" VERSION_LIST ${GUI_VERSION})
set(GUI_VERSION_HEX "0x")
foreach(_v ${VERSION_LIST})
	if (NOT ${_v} STREQUAL "0")
		string(APPEND GUI_VERSION_HEX "0${_v}")
	endif()
endforeach()
string(REPLACE "." "," GUI_VERSION_COMMA ${GUI_VERSION})

project(open-ephys-GUI)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
endif()
include(HelperFunctions.cmake)

set(BASE_BUILD_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Build)
set(JUCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode)
set(RESOURCES_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resources)

configure_file(${JUCE_DIRECTORY}/JuceHeader.h.in ${JUCE_DIRECTORY}/JuceHeader.h)
configure_file(${RESOURCES_DIRECTORY}/Build-files/resources.rc.in ${RESOURCES_DIRECTORY}/Build-files/resources.rc)

#global properties
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	JUCE_APP_VERSION=${GUI_VERSION}
	JUCE_APP_VERSION_HEX=${GUI_VERSION_HEX}
	)
	


#create executable and add JUCE components
add_executable(open-ephys 
	${JUCE_DIRECTORY}/AppConfig.h
	${JUCE_DIRECTORY}/JuceHeader.h
	${JUCE_DIRECTORY}/BinaryData.h
	${JUCE_DIRECTORY}/BinaryData.cpp
	${JUCE_DIRECTORY}/juce_audio_basics.cpp
	${JUCE_DIRECTORY}/juce_audio_devices.cpp
	${JUCE_DIRECTORY}/juce_audio_formats.cpp
	${JUCE_DIRECTORY}/juce_audio_processors.cpp
	${JUCE_DIRECTORY}/juce_audio_utils.cpp
	${JUCE_DIRECTORY}/juce_core.cpp
	${JUCE_DIRECTORY}/juce_cryptography.cpp
	${JUCE_DIRECTORY}/juce_data_structures.cpp
	${JUCE_DIRECTORY}/juce_events.cpp
	${JUCE_DIRECTORY}/juce_graphics.cpp
	${JUCE_DIRECTORY}/juce_gui_basics.cpp
	${JUCE_DIRECTORY}/juce_gui_extra.cpp
	${JUCE_DIRECTORY}/juce_opengl.cpp
	${JUCE_DIRECTORY}/juce_video.cpp
	${RESOURCES_DIRECTORY}/Build-files/resources.rc
	)



target_include_directories(open-ephys PRIVATE ${JUCE_DIRECTORY} ${JUCE_DIRECTORY}/modules)
target_compile_features(open-ephys PUBLIC cxx_auto_type cxx_generalized_initializers)
	
#output folders and specific options
if (MSVC OR LINUX)
set(BIN_PLUGIN_DIR $<TARGET_FILE_DIR:open-ephys>/plugins)
set(BIN_SHARED_DIR $<TARGET_FILE_DIR:open-ephys>/shared)
add_custom_command(TARGET open-ephys POST_BUILD  
		COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_SHARED_DIR}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_PLUGIN_DIR}"
		)
endif()

if(MSVC)
	set_property(TARGET open-ephys APPEND PROPERTY COMPILE_DEFINITIONS "JUCE_API=__declspec(dllexport)")
	target_compile_options(open-ephys PRIVATE /sdl- /nologo /MP)
	target_link_libraries(open-ephys setupapi.lib opengl32.lib glu32.lib)
	set_property(TARGET open-ephys APPEND_STRING PROPERTY LINK_FLAGS "/NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"msvcrt.lib\" /SUBSYSTEM:WINDOWS")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(DLLDIR "Win64")
	else()
		set(DLLDIR "Win32")
	endif()
	set(_copysrc "${RESOURCES_DIRECTORY}/DLLs/${DLLDIR}/*.dll" )
	file(GLOB _dllfiles ${_copysrc})
	add_custom_command(TARGET open-ephys POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy ${_dllfiles} ${BIN_SHARED_DIR}
		)
elseif(LINUX)
	set_property(TARGET open-ephys PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BASE_BUILD_DIRECTORY}/Linux)
elseif(APPLE)
	set_property(TARGET open-ephys PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BASE_BUILD_DIRECTORY}/XCode)
else()
	message( FATAL_ERROR "Unsupported OS")
endif()

#start recursive directory add
add_subdirectory(Source)

#create filters for vs and xcode
get_target_property(SRC_FILES open-ephys SOURCES)

foreach( src_file IN ITEMS ${SRC_FILES})
	get_filename_component(src_path "${src_file}" PATH)
	file(RELATIVE_PATH src_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${src_path}")
	string(REPLACE "/" "\\" group_name "${src_path_rel}")
	source_group("${group_name}" FILES "${src_file}")
endforeach()

#Add plugin build files
add_subdirectory(Plugins)
