cmake_minimum_required(VERSION 3.15)

include(../ComponentRules.cmake)

target_include_directories(${COMPONENT_NAME}_tests PRIVATE "${SOURCE_DIRECTORY}/Processors/DataThreads/")

add_sources(${COMPONENT_NAME}_tests
    DataBufferTests.cpp
    RecordNodeTests.cpp
    ProcessorGraphTests.cpp
)
