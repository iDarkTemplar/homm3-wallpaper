/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import homm3map 1.0

WallpaperItem {
	id: root

	readonly property var data_archives: root.configuration.DataArchives
	readonly property var map_list: root.configuration.MapList
	readonly property int refresh_time: root.configuration.RefreshTime
	readonly property int displayed_map_level: root.configuration.DisplayedMapLevel
	readonly property bool random_initial_posiion: root.configuration.RandomInitialPosition
	readonly property int initial_position_x: root.configuration.InitialPositionX
	readonly property int initial_position_y: root.configuration.InitialPositionY
	readonly property double scale: root.configuration.Scale

	readonly property int tile_size: 32

	Timer {
		id: reloadTimer
		interval: refresh_time * 1000
		running: false
		repeat: true

		onTriggered: {
			map.loadMap(chooseRandomMap(), chooseMapLevel());
		}
	}

	function chooseRandomMap()
	{
		if (map_list.length == 0)
		{
			return "";
		}

		var idx = Math.round(Math.random() * (map_list.length - 1));

		return map_list[idx].toString();
	}

	function chooseMapLevel()
	{
		if ((displayed_map_level >= 0) && (displayed_map_level <= 1))
		{
			return displayed_map_level;
		}

		return Math.round(Math.random());
	}

	Image {
		id: background
		anchors.fill: parent
		fillMode: Image.Tile

		Flickable {
			id: view
			anchors.fill: parent
			leftMargin: contentWidth >= width ? 0 : (width - contentWidth) / 2
			topMargin: contentHeight >= height ? 0 : (height - contentHeight) / 2
			contentItem.clip: true

			interactive: false

			contentWidth: map.width
			contentHeight: map.height

			Homm3Map {
				id: map

				scale: root.scale

				onLoadingFinished: {
					if (map.isMapLoaded())
					{
						if (random_initial_posiion)
						{
							if (view.contentWidth > view.width)
							{
								view.contentX = Math.round(Math.random() * (view.contentWidth - view.width) / (tile_size * root.scale)) * (tile_size * root.scale);
							}

							if (view.contentHeight > view.height)
							{
								view.contentY = Math.round(Math.random() * (view.contentHeight - view.height) / (tile_size * root.scale)) * (tile_size * root.scale);
							}
						}
						else
						{
							view.contentX = (initial_position_x + 1) * tile_size * root.scale;
							view.contentY = (initial_position_y + 1) * tile_size * root.scale;
						}
					}
					else
					{
						map.loadMap(chooseRandomMap(), chooseMapLevel());
					}
				}
			}
		}
	}

	Component.onCompleted: {
		map.setDataArchives(data_archives);
		background.source = "image://homm3/edg.def";

		map.loadMap(chooseRandomMap(), chooseMapLevel());

		if (refresh_time > 0)
		{
			reloadTimer.start();
		}
	}

	onScaleChanged: {
		// due to scale change, map needs to be reloaded. For now just load next map
		map.loadMap(chooseRandomMap(), chooseMapLevel());
	}
}
