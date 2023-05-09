
include(../ComponentRules.cmake)
add_sources(${COMPONENT_NAME}_tests
		DataViewportTests.cpp
)
target_include_directories(
		${COMPONENT_NAME}_tests
		PRIVATE
		"${SOURCE_DIRECTORY}"
)
