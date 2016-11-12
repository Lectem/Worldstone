project(WSsystem)

set(system_sources src/MpqArchive.cpp src/FileStream.cpp src/_VTablesTU.cpp)
set(system_headers include/Archive.h include/MpqArchive.h include/Stream.h include/IOBase.h include/FileStream.h src/FileStream.cpp)


add_library(ws_system ${system_sources} ${system_headers})
target_include_directories(ws_system
        PUBLIC include
        PRIVATE src)
target_link_libraries(ws_system
        PUBLIC fmt spdlog
        PRIVATE storm
        optimized enable_lto)
target_set_warnings(ws_system
        ENABLE ALL
        AS_ERROR ALL
        DISABLE Annoying)

# Always prefer to use namespaced aliases
add_library(WS::system ALIAS ws_system)
