if(NOT WS_WITH_BGFX)
    return()
endif()

find_package(SDL2)

if(NOT TARGET SDL2::SDL2)
    message(WARNING "SDL2 not found, not building RendererApp. If you already installed SDL2, please set SDL2DIR or CMAKE_PREFIX_PATH to your SDL2 installation.")
    return()
else()
    message(STATUS "Found SDL2 at ${SDL2_LIBRARY_DIR}, building RendererApp.")
endif()

add_subdirectory(imgui)

set(RendererAppWorkDir ${CMAKE_CURRENT_LIST_DIR}/workingDirectory)
set(ShaderBinariesDir ${RendererAppWorkDir}/shaders)

set(SRC_LIST
    main.cpp
    BaseApp.cpp
    RendererApp.cpp
    bgfxUtils.cpp
    DrawSprite.cpp
    SearchableListWidget.cpp
    FileBrowser.cpp
)
set(HDR_LIST
    main.h
    BaseApp.h
    RendererApp.h
    Inputs.h
    bgfxUtils.h
    DrawSprite.h
    SearchableListWidget.h
    FileBrowser.h
)

set(SHADERS_LIST
    shaders/vs_sprite.sc
    shaders/fs_sprite.sc
)

if( MSVC )
    set( SHADER_PLATFORMS dx11 glsl)
elseif( APPLE )
    set( SHADER_PLATFORMS glsl ) #metal)
else()
    set( SHADER_PLATFORMS glsl )
endif()

foreach(SHADER ${SHADERS_LIST})
    if(${SHADER} MATCHES "vs_.*")
        set(SHADER_TYPE VERTEX)
    elseif(${SHADER} MATCHES "fs_.*")
        set(SHADER_TYPE FRAGMENT)
    else()
        message(FATAL_ERROR "Unsupported shader name")
    endif()
    add_shader(${SHADER} ${SHADER_TYPE}
        OUTPUT ${ShaderBinariesDir}
        PLATFORMS ${SHADER_PLATFORMS}
        GLSL_VERSION 130
    )
endforeach()

add_executable(RendererApp
    WIN32
    ${SRC_LIST}
    ${HDR_LIST}
    ${SHADERS_LIST}
)
target_link_libraries(RendererApp
    PUBLIC
        WS::system
        WS::decoders
        SDL2::SDL2
        external::bgfx
        bgfx::imgui
)

set_target_properties(RendererApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${RendererAppWorkDir}
    FOLDER ${PROJECT_NAME}
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