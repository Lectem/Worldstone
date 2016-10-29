cmake_minimum_required(VERSION 3.6)	

add_subdirectory(decoders)

add_executable(DC6extract tools/DC6extract.cpp)
target_link_libraries(DC6extract decoders storm
						optimized enable_lto)

target_set_warnings(DC6extract decoders
					ENABLE ALL 
					AS_ERROR ALL
					DISABLE Annoying)
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT DC6extract)