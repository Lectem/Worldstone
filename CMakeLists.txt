cmake_minimum_required(VERSION 3.6)	

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# CMake modules
include("cmake/LTO.cmake")
include("cmake/Warnings.cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(Worldstone C CXX)

find_lto(CXX)

# External dependencies
add_subdirectory(external EXCLUDE_FROM_ALL)

# We want at least c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(source)