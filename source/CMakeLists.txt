cmake_minimum_required(VERSION 3.6)

add_subdirectory(decoders)
add_subdirectory(system)

add_executable(DC6extract tools/DC6extract.cpp)
target_link_libraries(DC6extract WS::decoders WS::system
						optimized enable_lto)

target_set_warnings(DC6extract
					ENABLE ALL
					AS_ERROR ALL
					DISABLE Annoying)


add_executable(MPQextract tools/MPQextract.cpp)
target_link_libraries(MPQextract WS::system
		optimized enable_lto)

target_set_warnings(MPQextract
		ENABLE ALL
		AS_ERROR ALL
		DISABLE Annoying)

add_subdirectory(tools/DCxViewer)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT DC6extract)
