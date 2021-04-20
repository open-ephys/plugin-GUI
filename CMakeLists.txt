#Open Ephys GUI main build file
cmake_minimum_required(VERSION 3.5.0)

set(GUI_VERSION 0.5.4)

string(REGEX MATCHALL "[0-9]+" VERSION_LIST ${GUI_VERSION})
set(GUI_VERSION_HEX "0x")
foreach(_v ${VERSION_LIST})
	if (NOT ${_v} STREQUAL "0")
		string(APPEND GUI_VERSION_HEX "0${_v}")
	endif()
endforeach()
string(REPLACE "." "," GUI_VERSION_COMMA ${GUI_VERSION})

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_SYSROOT "")
if ((NOT ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}/Build ) AND (NOT OE_DONT_CHECK_BUILD_PATH))
message(FATAL_ERROR "Please run cmake inside the Build folder to build the binaries in the predefined paths.\nSet the OE_DONT_CHECK_BUILD_PATH cmake variable to TRUE to disable this error")
endif()

project(open-ephys-GUI)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug)
	endif()
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
	$<$<PLATFORM_ID:Windows>:NOMINMAX>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	JUCE_APP_VERSION=${GUI_VERSION}
	JUCE_APP_VERSION_HEX=${GUI_VERSION_HEX}
	)

if (APPLE)
	set(JUCE_FILES_EXTENSION mm)
else()
	set(JUCE_FILES_EXTENSION cpp)
endif()


#create executable and add JUCE components
add_executable(open-ephys
	${JUCE_DIRECTORY}/AppConfig.h
	${JUCE_DIRECTORY}/JuceHeader.h
	${JUCE_DIRECTORY}/BinaryData.h
	${JUCE_DIRECTORY}/BinaryData.cpp
	${JUCE_DIRECTORY}/juce_audio_basics.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_audio_devices.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_audio_formats.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_audio_processors.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_audio_utils.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_core.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_cryptography.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_data_structures.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_events.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_graphics.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_gui_basics.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_gui_extra.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_opengl.${JUCE_FILES_EXTENSION}
	${JUCE_DIRECTORY}/juce_video.${JUCE_FILES_EXTENSION}
	)
 if(MSVC)
	target_sources(open-ephys PRIVATE ${RESOURCES_DIRECTORY}/Build-files/resources.rc)
 endif()


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

file(GLOB _bitfiles "${RESOURCES_DIRECTORY}/Bitfiles/*.bit")
add_custom_command(TARGET open-ephys POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${_bitfiles} $<TARGET_FILE_DIR:open-ephys>
		)

elseif(APPLE)
	set(BIN_PLUGIN_DIR $<TARGET_FILE_DIR:open-ephys>/../PlugIns)
	add_custom_command(TARGET open-ephys POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_PLUGIN_DIR}"
			)
endif()

if(MSVC)
	set_property(TARGET open-ephys APPEND PROPERTY COMPILE_DEFINITIONS "JUCE_API=__declspec(dllexport)")
	target_compile_options(open-ephys PRIVATE /sdl- /nologo /MP /W0)
	target_link_libraries(open-ephys setupapi.lib opengl32.lib glu32.lib)
	set_property(TARGET open-ephys APPEND_STRING PROPERTY LINK_FLAGS_DEBUG " /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"msvcrt.lib\"")
	set_property(TARGET open-ephys APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:WINDOWS")
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(DLLARCH "Win64")
	else()
		set(DLLARCH "Win32")
	endif()
	set(DLLDIR ${RESOURCES_DIRECTORY}/DLLs/${DLLARCH})
	file(GLOB _dllfiles "${DLLDIR}/*.dll")
	add_custom_command(TARGET open-ephys POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${_dllfiles} ${BIN_SHARED_DIR}
		)
elseif(LINUX)
	if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

	elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	else()
		message(FATAL_ERROR "Invalid build type")
	endif()
	find_package(CURL REQUIRED)
	target_include_directories(open-ephys PRIVATE
		/usr/include
		/usr/include/freetype2
		${CURL_INCLUDE_DIR}
	)
	target_link_libraries(open-ephys GL X11 Xext Xinerama asound dl freetype pthread rt ${CURL_LIBRARIES})
	set_property(TARGET open-ephys APPEND_STRING PROPERTY LINK_FLAGS
		"-fvisibility=hidden -fPIC -rdynamic -Wl,-rpath,'$ORIGIN/shared'")
	target_compile_options(open-ephys PRIVATE -fPIC -rdynamic -fvisibility=hidden)
	target_compile_options(open-ephys PRIVATE -O3) #enable optimization for linux debug
	set_property(TARGET open-ephys PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BASE_BUILD_DIRECTORY}/${CMAKE_BUILD_TYPE})

	set(DLLDIR ${RESOURCES_DIRECTORY}/DLLs/Linux)
	file(GLOB _dllfiles "${DLLDIR}/*.so")
	add_custom_command(TARGET open-ephys POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${_dllfiles} ${BIN_SHARED_DIR}
		)

elseif(APPLE)
	set_target_properties(open-ephys PROPERTIES
		MACOSX_BUNDLE_NAME open-ephys
		MACOSX_BUNDLE_VERSION ${GUI_VERSION}
		MACOSX_BUNDLE_GUI_IDENTIFIER "org.open-ephys.gui"
		MACOSX_BUNDLE_ICON_FILE "Icon.icns"
		MACOSX_BUNDLE_INFO_PLIST "${RESOURCES_DIRECTORY}/Build-files/Info.plist.in"
		MACOSX_BUNDLE TRUE
		)
	target_compile_options(open-ephys PRIVATE -fPIC -Wpartial-availability -Wno-inconsistent-missing-override -rdynamic)
	set_property(TARGET open-ephys APPEND_STRING PROPERTY LINK_FLAGS "-fPIC -rdynamic")
	target_link_libraries(open-ephys dl)
	target_link_libraries(open-ephys
		"-framework Accelerate"
		"-framework AudioToolbox"
		"-framework Carbon"
		"-framework Cocoa"
		"-framework CoreAudio"
		"-framework CoreMIDI"
		"-framework DiscRecording"
		"-framework IOKit"
		"-framework OpenGL"
		"-framework QTKit"
		"-framework QuartzCore"
		"-framework WebKit"
	)

	set(DLLDIR ${RESOURCES_DIRECTORY}/DLLs/Mac)

	set_target_properties(open-ephys PROPERTIES
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
		XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
		XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf
		XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN YES
		XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO
		)

		set(MAC_RESOURCE_FILES
			${RESOURCES_DIRECTORY}/Build-files/Icon.icns
			${RESOURCES_DIRECTORY}/Build-files/dmg_background.png
			${RESOURCES_DIRECTORY}/Build-files/RecentFilesMenuTemplate.nib
			${RESOURCES_DIRECTORY}/Bitfiles/rhd2000.bit
			${RESOURCES_DIRECTORY}/Bitfiles/rhd2000_usb3.bit
			${RESOURCES_DIRECTORY}/Fonts/unibody-8.otf
		)
		set_source_files_properties(${MAC_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
		target_sources(open-ephys PRIVATE ${MAC_RESOURCE_FILES})

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
