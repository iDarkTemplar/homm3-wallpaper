/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
	id: window
	width: 640
	height: 480
	visible: true
	visibility: "Maximized"
	title: qsTr("HOMM 3 Wallpaper Viewer")

	background: Image {
		source: "image://homm3/edg.def"
		fillMode: Image.Tile
	}
}
