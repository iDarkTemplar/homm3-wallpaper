cmake_minimum_required( VERSION 3.18.0 )

project(homm3-wallpaper
	VERSION 0.1.0
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(QML_PLUGIN_NAME "homm3map")

find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)

find_package(Qt6 COMPONENTS Core Gui OpenGL Quick REQUIRED)
find_package(Plasma 6.1.2 REQUIRED)

option(VIEWER "Build HOMM3 map viewer application" true)

if (VIEWER)
	find_package(Qt6 COMPONENTS Widgets QuickWidgets REQUIRED)
endif (VIEWER)

set(LIBRARY_SOURCES
	data_maps.cpp
	def_file.cpp
	homm3_image_provider.cpp
	homm3map.cpp
	homm3singleton.cpp
	lod_archive.cpp
	random.cpp
	texture_atlas.cpp
	vcmi/CBinaryReader.cpp
	vcmi/CCompressedStream.cpp
	vcmi/CFileInputStream.cpp
	vcmi/CMap.cpp
	vcmi/MapFormatH3M.cpp
	vcmi/ObjectTemplate.cpp
	)

set(LIBRARY_HEADERS
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

set(APP_SOURCES
	main.cpp
	mainwindow.cpp
	settingswindow.cpp
	)

set(APP_HEADERS
	mainwindow.h
	settingswindow.h
	)

set(APP_UI
	mainwindow.ui
	settingswindow.ui
	)

set(APP_RESOURCES
	qml.qrc
	)

set(PLUGIN_SOURCES
	plugin.cpp
	)

set(PLUGIN_HEADERS
	plugin.h
	)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIR})

qt_wrap_cpp(MOC_LIBRARY_HEADERS ${LIBRARY_HEADERS})
qt_wrap_cpp(MOC_PLUGIN_HEADERS ${PLUGIN_HEADERS})

add_library(homm3map STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS} ${MOC_LIBRARY_HEADERS})
target_link_libraries(homm3map ZLIB::ZLIB Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::Quick)

if (VIEWER)
	qt_add_resources(COMPILED_APP_RESOURCES ${APP_RESOURCES})
	qt_wrap_cpp(MOC_APP_HEADERS ${APP_HEADERS})
	qt_wrap_ui(CPP_APP_UI ${APP_UI})

	add_executable(homm3map-viewer ${APP_SOURCES} ${APP_HEADERS} ${CPP_APP_UI} ${MOC_APP_HEADERS} ${COMPILED_APP_RESOURCES})
	target_include_directories(homm3map-viewer PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(homm3map-viewer homm3map Qt6::Gui Qt6::Widgets Qt6::Quick Qt6::QuickWidgets)
endif (VIEWER)

add_library(homm3mapplugin MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} ${MOC_PLUGIN_HEADERS})
target_link_libraries(homm3mapplugin homm3map Qt6::Quick)

set_target_properties(homm3mapplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/import/${QML_PLUGIN_NAME})
add_custom_target(copy_qmldir
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/plugin/qmldir ${CMAKE_CURRENT_BINARY_DIR}/import/${QML_PLUGIN_NAME}/qmldir
	SOURCES plugin/qmldir
	DEPENDS plugin/qmldir)

add_dependencies(homm3mapplugin copy_qmldir)

add_custom_target(wallpaper
	SOURCES
	wallpaper/contents/config/main.xml
	wallpaper/contents/ui/config.qml
	wallpaper/contents/ui/main.qml
	wallpaper/metadata.json
	)

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML OUTPUT_VARIABLE QT_INSTALL_QML OUTPUT_STRIP_TRAILING_WHITESPACE)

include(GNUInstallDirs)

if (VIEWER)
	install(TARGETS homm3map-viewer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif (VIEWER)

install(TARGETS homm3mapplugin LIBRARY DESTINATION ${QT_INSTALL_QML}/${QML_PLUGIN_NAME})
install(FILES plugin/qmldir DESTINATION ${QT_INSTALL_QML}/${QML_PLUGIN_NAME})

plasma_install_package(wallpaper homm3mapwallpaper wallpapers wallpaper)
