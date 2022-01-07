/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import homm3map 1.0

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

	MouseArea {
		anchors.fill: parent
		acceptedButtons: Qt.RightButton

		onClicked: {
			map.toggleLevel();
		}

		Flickable {
			id: root

			anchors.fill: parent
			leftMargin: contentWidth >= width ? 0 : (width - contentWidth) / 2
			topMargin: contentHeight >= height ? 0 : (height - contentHeight) / 2
			contentItem.clip: true

			contentWidth: map.width
			contentHeight: map.height

			Homm3Map {
				id: map
			}
		}
	}

	Component.onCompleted: {
		if (typeof map_name !== "undefined" && map_name != "")
		{
			map.loadMap(map_name);
		}
	}
}
