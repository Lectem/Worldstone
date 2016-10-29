cmake_minimum_required(VERSION 3.6)
project(decoders)

set(DECODERS_SOURCES dc6.cpp)
set(DECODERS_HEADERS dc6.h utils.cpp utils.h palette.cpp palette.h)
add_library(decoders ${DECODERS_SOURCES} ${DECODERS_HEADERS})
target_include_directories(decoders PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(decoders PUBLIC fmt spdlog
							optimized enable_lto)