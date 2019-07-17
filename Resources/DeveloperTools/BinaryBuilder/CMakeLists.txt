cmake_minimum_required(VERSION 3.5.0)
project(BinaryBuilder)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug)
	endif()
endif()

if (APPLE)
	set(JUCE_FILES_EXTENSION mm)
else()
	set(JUCE_FILES_EXTENSION cpp)
endif()

add_executable(BinaryBuilder Source/Main.cpp JuceLibraryCode/include_juce_core.${JUCE_FILES_EXTENSION})


target_compile_definitions(BinaryBuilder PRIVATE
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Windows>:_CONSOLE>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	JUCE_APP_VERSION="1.0.0"
	JUCE_APP_VERSION_HEX="0x10000"
	JucePlugin_Build_VST=0
	JucePlugin_Build_VST3=0
	JucePlugin_Build_AU=0
	JucePlugin_Build_AUv3=0
	JucePlugin_Build_RTAS=0
	JucePlugin_Build_AAX=0
	JucePlugin_Build_Standalone=0
	JucePlugin_Build_Unity=0
	)

target_include_directories(BinaryBuilder PRIVATE JuceLibraryCode JuceLibraryCode/modules)
target_compile_features(BinaryBuilder PUBLIC cxx_auto_type cxx_generalized_initializers)

if(MSVC)
	target_compile_options(BinaryBuilder PRIVATE /sdl- /nologo /MP)
	set_property(TARGET BinaryBuilder APPEND_STRING PROPERTY LINK_FLAGS_DEBUG " /NODEFAULTLIB:\"libcmt.lib\" /NODEFAULTLIB:\"msvcrt.lib\"")
	set_property(TARGET BinaryBuilder APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
elseif(LINUX)
	find_package(CURL REQUIRED)
	target_compile_options(BinaryBuilder PRIVATE -pthread)
	target_link_libraries(BinaryBuilder dl pthread ${CURL_LIBRARIES})
	target_include_directories(BinaryBuilder PRIVATE ${CURL_INCLIDE_DIR})
elseif(APPLE)
	target_link_libraries(BinaryBuilder dl)
	target_link_libraries(BinaryBuilder
		"-framework Cocoa"
		"-framework IOKit"
	)
endif()
