project(tools)

add_executable(DC6extract DC6extract.cpp)
target_link_libraries(DC6extract
    PUBLIC
    WS::decoders WS::system
)
target_enable_lto(DC6extract optimized)

target_set_warnings(DC6extract
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)


add_executable(MPQextract MPQextract.cpp)
target_link_libraries(MPQextract
    PUBLIC
    WS::system
)
target_enable_lto(MPQextract optimized)

target_set_warnings(MPQextract
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)

set_target_properties(DC6extract MPQextract PROPERTIES FOLDER ${PROJECT_NAME})

add_subdirectory(DCxViewer)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT DC6extract)
