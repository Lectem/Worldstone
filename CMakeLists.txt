cmake_minimum_required(VERSION 3.6)	

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW) # When INTERPROCEDURAL_OPTIMIZATION is set, give an error if IPO is not supported
endif()

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(LTO)
include(Warnings)
include(RunFixupBundle)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(Worldstone C CXX)

find_lto(CXX)

# External dependencies
add_subdirectory(external EXCLUDE_FROM_ALL)

# We want at least c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(source)