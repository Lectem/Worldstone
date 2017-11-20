project(decoders)

set(DECODERS_SOURCES
    src/dc6.cpp
    src/dcc.cpp
    src/palette.cpp
    src/utils.cpp
)

set(DECODERS_HEADERS
    include/AABB.h
    include/dc6.h
    include/dcc.h
    include/utils.h
    include/palette.h
)

add_library(ws_decoders ${DECODERS_SOURCES} ${DECODERS_HEADERS})
target_include_directories(ws_decoders
    PUBLIC include
    PRIVATE src
)
target_link_libraries(ws_decoders
    PUBLIC WS::system
)
target_enable_lto(ws_decoders optimized)
target_set_warnings(ws_decoders
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)


add_library(WS::decoders ALIAS ws_decoders)

add_subdirectory(tests)