project(WSsystem)

set(system_sources
    src/MpqArchive.cpp
    src/FileStream.cpp
    src/BitStream.cpp
    src/_VTablesTU.cpp
)
set(system_headers
    include/Archive.h
    include/IOBase.h
    include/Log.h
    include/MpqArchive.h
    include/Stream.h
    include/FileStream.h
    include/BitStream.h
    include/SystemUtils.h
    src/FileStream.cpp
)


add_library(ws_system ${system_sources} ${system_headers})
target_include_directories(ws_system
    PUBLIC include
    PRIVATE src)
target_link_libraries(ws_system
    PUBLIC external::fmt external::spdlog
    PRIVATE external::storm
)
target_enable_lto(ws_system optimized)
target_set_warnings(ws_system
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)

# Always prefer to use namespaced aliases
add_library(WS::system ALIAS ws_system)

add_subdirectory(tests)