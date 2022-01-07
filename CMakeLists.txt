cmake_minimum_required( VERSION 3.18.0 )

project(homm3-wallpaper
	VERSION 0.1.0
	LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)

set (SOURCES
	def_file.cpp
	main.cpp
	lod_archive.cpp
	random.cpp
	vcmi/CBinaryReader.cpp
	vcmi/CCompressedStream.cpp
	vcmi/CFileInputStream.cpp
	)

set(HEADERS
	def_file.h
	globals.h
	lod_archive.h
	random.h
	vcmi/CBinaryReader.h
	vcmi/CCompressedStream.h
	vcmi/CFileInputStream.h
	vcmi/CInputStream.h
	vcmi/CStream.h
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})

add_executable(homm3-wallpaper ${SOURCES} ${HEADERS})
target_link_libraries(homm3-wallpaper ZLIB::ZLIB)

include(GNUInstallDirs)

install(TARGETS homm3-wallpaper RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
