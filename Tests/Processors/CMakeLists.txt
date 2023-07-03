include(../ComponentRules.cmake)
add_sources(${COMPONENT_NAME}_tests
		RecordNodeTests.cpp
		ProcessorGraphTests.cpp
)
target_include_directories(
		${COMPONENT_NAME}_tests
		PRIVATE
		"${SOURCE_DIRECTORY}"
)

target_compile_definitions(
		${COMPONENT_NAME}_tests
		PRIVATE
		RESOURCES_DIRECTORY="${RESOURCES_DIRECTORY}"
)
