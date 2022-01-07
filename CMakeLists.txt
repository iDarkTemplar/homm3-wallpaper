cmake_minimum_required( VERSION 3.18.0 )

project(homm3-wallpaper
	VERSION 0.1.0
	LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)

find_package(Qt5 COMPONENTS Core Quick REQUIRED)

set (SOURCES
	data_maps.cpp
	def_file.cpp
	homm3_image_provider.cpp
	main.cpp
	lod_archive.cpp
	random.cpp
	vcmi/CBinaryReader.cpp
	vcmi/CCompressedStream.cpp
	vcmi/CFileInputStream.cpp
	)

set(HEADERS
	data_maps.h
	def_file.h
	globals.h
	homm3_image_provider.h
	lod_archive.h
	random.h
	vcmi/CBinaryReader.h
	vcmi/CCompressedStream.h
	vcmi/CFileInputStream.h
	vcmi/CInputStream.h
	vcmi/CStream.h
	vcmi/GameConstants.h
	)

set(RESOURCES
	qml.qrc
)

qt_add_resources(COMPILED_RESOURCES ${RESOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})

add_executable(homm3-wallpaper ${SOURCES} ${HEADERS} ${COMPILED_RESOURCES})
target_link_libraries(homm3-wallpaper ZLIB::ZLIB Qt5::Core Qt5::Quick)

include(GNUInstallDirs)

install(TARGETS homm3-wallpaper RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
