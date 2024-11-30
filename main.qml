/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import homm3mapprivate 1.0

Item {
	Image {
		anchors.fill: parent
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
			anchors.fill: parent
			leftMargin: contentWidth >= width ? 0 : (width - contentWidth) / 2
			topMargin: contentHeight >= height ? 0 : (height - contentHeight) / 2
			contentItem.clip: true

			contentWidth: map.width
			contentHeight: map.height

			Homm3Map {
				id: map
				objectName: "map"
			}
		}
	}
}
