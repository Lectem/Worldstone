project(external C CXX)

set(BUILD_SHARED_LIBS_BCKP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force disable of shared libraries" FORCE)

set(BUILD_TESTING_BCKP ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Force disable of tests for external dependencies" FORCE)

set(CUSTOM_COVERAGE_EXCLUDE ${CUSTOM_COVERAGE_EXCLUDE} "external" PARENT_SCOPE) # Replaced in CTestCustom.cmake.in

#Use fmtlib for formatting
add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_library(external::fmt ALIAS fmt)

#Use spdlog for logging, and tell it to use our version of fmtlib
add_subdirectory(spdlog EXCLUDE_FROM_ALL)
target_compile_definitions(spdlog INTERFACE SPDLOG_FMT_EXTERNAL)
add_library(external::spdlog ALIAS spdlog)

#Doctest for unit tests
add_library(doctest INTERFACE)
target_include_directories(
    doctest
    INTERFACE
    doctest/doctest # note : will expose the parts/ folder...
)
add_library(external::doctest ALIAS doctest)

add_subdirectory(StormLib EXCLUDE_FROM_ALL)
target_set_warnings(storm DISABLE ALL)
add_library(external::storm ALIAS storm)

#Add new external (non-interface) targets to this list
set_target_properties(fmt storm PROPERTIES FOLDER "external")

if(WS_WITH_BGFX)
    option( BGFX_BUILD_TOOLS      "Build bgfx tools."                             ON  )
    option( BGFX_BUILD_EXAMPLES   "Build bgfx examples."                          OFF )
    option( BGFX_INSTALL          "Create installation target."                   OFF )
    option( BGFX_INSTALL_EXAMPLES "Install examples and their runtimes."          OFF )
    option( BGFX_CUSTOM_TARGETS   "Include convenience custom targets."           OFF )
    option( BGFX_USE_OVR          "Build with OVR support."                       OFF )
    option( BGFX_AMALGAMATED      "Amalgamated bgfx build for faster compilation" ON  )
    option( BX_AMALGAMATED        "Amalgamated bx build for faster compilation"   ON  )
    option( BGFX_CONFIG_DEBUG     "Enables debug configuration on all builds"     OFF )

    add_subdirectory(bgfx.cmake)
    target_set_warnings(bgfx DISABLE ALL)
    add_library(external::bgfx ALIAS bgfx)
    include( bgfx.cmake/cmake/3rdparty/dear-imgui.cmake )
    target_set_warnings(dear-imgui DISABLE ALL)
    add_library(imgui::imgui ALIAS dear-imgui)
endif()

set(BUILD_TESTING ${BUILD_TESTING_BCKP} CACHE BOOL "Build tests (default variable for CTest)" FORCE) #Set it back to its past value
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_BCKP} CACHE BOOL "Cause all libraries to be built shared unless the library was explicitly added as a static library." FORCE)
