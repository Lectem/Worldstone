cmake_minimum_required(VERSION 3.8.2)

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW) # When INTERPROCEDURAL_OPTIMIZATION is set, give an error if IPO is not supported
endif()

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

project(Worldstone C CXX)

############################
##  Modules and scripts   ##
############################

# Standard CMake modules

include(CTest) # Must be called before adding tests but after calling project(). This automatically calls enable_testing() and configures ctest targets when using Make/Ninja
include(CMakeDependentOption)# This is a really useful scripts that creates options that depends on other options. It can even be used with generator expressions !

# Custom modules and scripts

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(LTO)
include(Warnings)
include(CopyDllsForDebug)
include(Coverage)

###############
##  OPTIONS  ##
###############

option(WS_WITH_BGFX "Build target that require bgfx" ON)
option(WS_USE_DOXYGEN "Add a doxygen target to generate the documentation" ON)

# Use our own option for tests, in case people use our libraries through add_subdirectory
cmake_dependent_option(WS_BUILD_TESTS
    "Enable Worldstone project tests targets" ON # By default we want tests if CTest is enabled
    "BUILD_TESTING" OFF # Stay coherent with CTest variables
)

# Let the user add options to the test runner if needed
set(TEST_RUNNER_PARAMS "--force-colors=true" CACHE STRING "Options to add to our test runners commands")

# Global project setup

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_lto(CXX)

# External dependencies
add_subdirectory(external EXCLUDE_FROM_ALL)

# We want at least c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# TODO : replace by target_compile_features(xxx PRIVATE cxx_std_14) now that we use 3.8.2

# Delegate the target setup to source/CMakeLists.txt
add_subdirectory(source)

#===========#
#   Tests   #
#===========#

if(WS_BUILD_TESTS)
    # Exclude test folders from coverage, keep in sync with .codecov.yml
    list(APPEND CUSTOM_COVERAGE_EXCLUDE "/tests/")

    # You can setup some custom variables and add them to the CTestCustom.cmake.in template to have custom ctest settings
    # For example, you can exclude some directories from the coverage reports such as third-parties and tests
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/CTestCustom.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
        @ONLY
    )
endif()

#############
## Doxygen ##
#############

if(WS_USE_DOXYGEN AND ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER_EQUAL 3.9)
    find_package(Doxygen
        OPTIONAL_COMPONENTS dot mscgen dia
    )
    if(DOXYGEN_FOUND)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
        set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
        set(DOXYGEN_SOURCE_BROWSER YES)
        set(DOXYGEN_GENERATE_TREEVIEW YES)
        # Doxygen macros
        set(DOXYGEN_MACRO_EXPANSION YES)
        set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
        set(DOXYGEN_PREDEFINED [[TYPE_TO_STRING(x)=""]])
        # Custom commands
        # The following will expand to
        set(DOXYGEN_ALIASES
            # Using \t instead of spaces because of https://gitlab.kitware.com/cmake/cmake/issues/17489
            #test{2}="@xrefitem	\1_tests	\"Tests\"	\"\1:TestsList\"	@ref	TEST_\2	\"\2\""
            ;test{2}="@xrefitem\t\\1_tests\t\\\"Tests\\\"\t\\\"\\1:TestsList\\\"\t@ref\tTEST_\\2\t\\\"\\2\\\""
            #testimpl{1}="@anchor	TEST_\1"
            ;testimpl{1}="@anchor\tTEST_\\1"
            #testimpl{2}="@anchor	TEST_\2	@sa	\1"
            ;testimpl{2}="@anchor\tTEST_\\2\t@sa\t\\1"
        )
        # Doxygen spits way too much of these warnings...
        set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
        doxygen_add_docs(
            doc
            README.md source doc
            COMMENT "Generate man pages"
        )
    endif()
endif()
