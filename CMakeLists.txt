cmake_minimum_required( VERSION 3.18.0 )

project(homm3-wallpaper
	VERSION 0.1.0
	LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)

find_package(Qt5 COMPONENTS Core Gui Concurrent Quick REQUIRED)

set (SOURCES
	data_maps.cpp
	def_file.cpp
	homm3_image_provider.cpp
	homm3map.cpp
	homm3singleton.cpp
	lod_archive.cpp
	main.cpp
	random.cpp
	texture_atlas.cpp
	vcmi/CBinaryReader.cpp
	vcmi/CCompressedStream.cpp
	vcmi/CFileInputStream.cpp
	vcmi/CMap.cpp
	vcmi/MapFormatH3M.cpp
	vcmi/ObjectTemplate.cpp
	)

set(HEADERS
	data_maps.h
	def_file.h
	globals.h
	homm3_image_provider.h
	homm3map.h
	homm3singleton.h
	lod_archive.h
	random.h
	texture_atlas.h
	vcmi/CBinaryReader.h
	vcmi/CCompressedStream.h
	vcmi/CFileInputStream.h
	vcmi/CInputStream.h
	vcmi/CGTownInstance.h
	vcmi/CMap.h
	vcmi/CMapDefines.h
	vcmi/CObjectHandler.h
	vcmi/CQuest.h
	vcmi/CStream.h
	vcmi/GameConstants.h
	vcmi/MapFormatH3M.h
	vcmi/ObjectTemplate.h
	vcmi/int3.h
	)

set(RESOURCES
	qml.qrc
)

qt_add_resources(COMPILED_RESOURCES ${RESOURCES})
qt_wrap_cpp(MOC_HEADERS ${HEADERS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})

add_executable(homm3-wallpaper ${SOURCES} ${HEADERS} ${MOC_HEADERS} ${COMPILED_RESOURCES})
target_link_libraries(homm3-wallpaper ZLIB::ZLIB Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::Quick)

include(GNUInstallDirs)

install(TARGETS homm3-wallpaper RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
