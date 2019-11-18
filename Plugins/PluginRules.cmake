#common options for default plug-ins
unset(PROJECT_FOLDER)
unset(PLUGIN_NAME)
get_filename_component(PROJECT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)
get_filename_component(PLUGIN_NAME ${PROJECT_FOLDER} NAME)

if (APPLE)
	add_library(${PLUGIN_NAME} MODULE OpenEphysLib.cpp)
else()
	add_library(${PLUGIN_NAME} SHARED OpenEphysLib.cpp)
endif()

add_dependencies(${PLUGIN_NAME} open-ephys)
target_include_directories(${PLUGIN_NAME} PRIVATE ${JUCE_DIRECTORY} ${JUCE_DIRECTORY}/modules ${PLUGIN_HEADER_PATH})
target_compile_features(${PLUGIN_NAME} PUBLIC cxx_auto_type cxx_generalized_initializers)

#Libraries and compiler options
if(MSVC)
	target_link_libraries(${PLUGIN_NAME} $<TARGET_FILE_DIR:open-ephys>/open-ephys.lib)
	target_compile_options(${PLUGIN_NAME} PRIVATE /sdl-)
elseif(LINUX)
	target_link_libraries(${PLUGIN_NAME} GL X11 Xext Xinerama asound dl freetype pthread rt)
	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		"-fvisibility=hidden -fPIC -rdynamic -Wl,-rpath,'$ORIGIN/../shared'")
	target_compile_options(${PLUGIN_NAME} PRIVATE -fPIC -rdynamic)
	target_compile_options(${PLUGIN_NAME} PRIVATE -O3) #enable optimization for linux debug
elseif(APPLE)
	set_target_properties(${PLUGIN_NAME} PROPERTIES
		BUNDLE TRUE
		MACOSX_BUNDLE_GUI_IDENTIFIER "org.open-ephys.plugin.${PLUGIN_NAME}"
	)

	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-fPIC -rdynamic -undefined dynamic_lookup")
	target_link_libraries(${PLUGIN_NAME} dl)

	set_target_properties(${PLUGIN_NAME} PROPERTIES
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
		XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO
		XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT dwarf
		XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN YES
		XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO
		)
endif()

#output folders
set_property(TARGET ${PLUGIN_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BIN_PLUGIN_DIR})
set_property(TARGET ${PLUGIN_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BIN_PLUGIN_DIR})


#This function is to be called to organize filters in VisualStudio and XCode in plugins with subfilders
function(plugin_create_filters)
get_target_property(PLUGIN_SRC_FILES ${PLUGIN_NAME} SOURCES)

foreach( plugin_src_file IN ITEMS ${PLUGIN_SRC_FILES})
	if (NOT ${plugin_src_file} STREQUAL "OpenEphysLib.cpp")
		get_filename_component(plugin_src_path "${plugin_src_file}" PATH)
		file(RELATIVE_PATH plugin_src_path_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${plugin_src_path}")
		string(REPLACE "/" "\\" plugin_group_name "${plugin_src_path_rel}")
		source_group("${plugin_group_name}" FILES "${plugin_src_file}")
	endif()
endforeach()
endfunction()
