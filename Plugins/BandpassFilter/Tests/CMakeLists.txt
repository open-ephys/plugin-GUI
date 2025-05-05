cmake_minimum_required(VERSION 3.15)

add_sources(${PLUGIN_NAME}_tests BandpassFilterTests.cpp)
target_include_directories(${PLUGIN_NAME}_tests PRIVATE "${SOURCE_DIRECTORY}")
