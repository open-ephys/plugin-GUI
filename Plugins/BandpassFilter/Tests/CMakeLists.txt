cmake_minimum_required(VERSION 3.5.0)

add_sources(${PLUGIN_NAME}_tests BandpassFilterTests.cpp)
target_include_directories(${PLUGIN_NAME}_tests PRIVATE "${SOURCE_DIRECTORY}")
