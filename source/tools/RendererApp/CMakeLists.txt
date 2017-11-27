project(RendererApp)

find_package(SDL2)

if(NOT TARGET SDL2::SDL2)
    message(WARNING "SDL2 not found, not building RendererApp. If you already installed SDL2, please set SDL2DIR or CMAKE_PREFIX_PATH to your SDL2 installation.")
    return()
else()
    message(STATUS "Found SDL2 at ${SDL2_LIBRARY_DIR}, building RendererApp.")
endif()

set(SRC_LIST main.cpp)
set(HDR_LIST main.h)

add_executable(RendererApp WIN32 ${SRC_LIST} ${HDR_LIST})
target_link_libraries(RendererApp
    PUBLIC
        WS::system
        WS::decoders
        SDL2::SDL2
)

target_enable_lto(RendererApp optimized)

target_set_warnings(RendererApp
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)

get_target_property(SDL2_location SDL2::SDL2 LOCATION)
get_filename_component(SDL2_directory ${SDL2_location} DIRECTORY)
copy_dlls_for_debug(RendererApp "" "${SDL2_directory}")