if(NOT WS_WITH_QT)
    return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Quick Widgets QUIET)
find_package(Qt5QuickCompiler QUIET)

if(Qt5_FOUND)
	set(SRC_LIST main.cpp DCxMainWindow.cpp DCxView.cpp)
	set(HDR_LIST main.h DCxMainWindow.h DCxView.h)
	if(Qt5QuickCompiler_FOUND)
		qtquick_compiler_add_resources(RESOURCES main.qrc)
	else()
		qt5_add_resources(RESOURCES main.qrc)
	endif()


	add_executable(DCxViewer WIN32 ${SRC_LIST} ${HDR_LIST} ${RESOURCES})
	target_link_libraries(DCxViewer Qt5::Quick Qt5::Widgets WS::system WS::decoders)

	get_target_property(QtCore_location Qt5::Core LOCATION)
	get_filename_component(QtCore_directory ${QtCore_location} DIRECTORY)
	copy_dlls_for_debug(DCxViewer "" ${QtCore_directory})

	set_target_properties(DCxViewer PROPERTIES FOLDER ${PROJECT_NAME})

	message(STATUS "Found Qt5 at ${Qt5_DIR}, building DCxViewer.")
else()
	message(WARNING "Will not build DCxViewer since Qt5 can't be found. Please add your Qt5 path (For example C:/Qt/5.7/msvc2015_64 on windows) to CMAKE_PREFIX_PATH")
endif()