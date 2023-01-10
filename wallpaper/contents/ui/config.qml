/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.0
import org.kde.kcm 1.5 as KCM
import org.kde.kirigami 2.12 as Kirigami

ColumnLayout {
	id: root

	property var cfg_DataArchives: []
	property var cfg_DataArchivesDefault: []
	property var cfg_MapList: []
	property var cfg_MapListDefault: []
	property int cfg_RefreshTime
	property int cfg_RefreshTimeDefault: 900
	property int cfg_DisplayedMapLevel
	property int cfg_DisplayedMapLevelDefault: -1
	property bool cfg_RandomInitialPosition
	property bool cfg_RandomInitialPositionDefault: true
	property int cfg_InitialPositionX
	property int cfg_InitialPositionXDefault: -1
	property int cfg_InitialPositionY
	property int cfg_InitialPositionYDefault: -1
	property double cfg_Scale
	property double cfg_ScaleDefault: 1.0

	property int hoursIntervalValue: Math.floor(cfg_RefreshTime / 3600)
	property int minutesIntervalValue: Math.floor((cfg_RefreshTime % 3600) / 60)
	property int secondsIntervalValue: cfg_RefreshTime % 60

	property int hoursIntervalValueDefault: Math.floor(cfg_RefreshTimeDefault / 3600)
	property int minutesIntervalValueDefault: Math.floor((cfg_RefreshTimeDefault % 3600) / 60)
	property int secondsIntervalValueDefault: cfg_RefreshTimeDefault % 60

	SystemPalette {
		id: syspal
	}

	FileDialog {
		id: archive_dialog
		visible: false
		selectMultiple: true
		title: i18nd("homm3mapwallpaper", "Please choose a file")
		folder: shortcuts.home

		onAccepted: {
			cfg_DataArchives = cfg_DataArchives.concat(archive_dialog.fileUrls);
		}
	}

	FileDialog {
		id: map_dialog
		visible: false
		selectMultiple: true
		title: i18nd("homm3mapwallpaper", "Please choose a file")
		folder: shortcuts.home

		onAccepted: {
			cfg_MapList = cfg_MapList.concat(map_dialog.fileUrls);
		}
	}

	Kirigami.FormLayout {
		twinFormLayouts: parentLayout

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "HOMM3 data archives (usually H3sprite.lod and H3ab_spr.lod):")

			Rectangle {
				color: syspal.base
				width: data_archives_view.width
				height: data_archives_view.height

				ListView {
					visible: true
					id: data_archives_view
					implicitWidth: 800
					implicitHeight: 100
					clip: true
					model: cfg_DataArchives
					delegate: Text {
						text: modelData
					}
					ScrollBar.vertical: ScrollBar {
						active: true
					}
				}
			}
		}

		RowLayout {
			Button {
				icon.name: "list-add"
				text: i18nd("homm3mapwallpaper","Add data archives")
				onClicked: archive_dialog.open();
			}

			Button {
				icon.name: "list-remove"
				text: i18nd("homm3mapwallpaper","Remove all data archives")
				onClicked: {
					cfg_DataArchives = [];
				}
			}
		}

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "HOMM3 maps:")

			Rectangle {
				color: syspal.base
				width: maps_view.width
				height: maps_view.height

				ListView {
					visible: true
					id: maps_view
					implicitWidth: 800
					implicitHeight: 200
					clip: true
					model: cfg_MapList
					delegate: Text {
						text: modelData
					}
					ScrollBar.vertical: ScrollBar {
						active: true
					}
				}
			}
		}

		RowLayout {
			Button {
				icon.name: "list-add"
				text: i18nd("homm3mapwallpaper","Add map files")
				onClicked: map_dialog.open();
			}

			Button {
				icon.name: "list-remove"
				text: i18nd("homm3mapwallpaper","Remove all map files")
				onClicked: {
					cfg_MapList = [];
				}
			}
		}

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "Change every:")

			SpinBox {
				id: hoursInterval
				value: root.hoursIntervalValue
				from: 0
				to: 24
				editable: true
				onValueChanged: cfg_RefreshTime = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

				textFromValue: function(value, locale) {
					return i18ndp("homm3mapwallpaper","%1 hour", "%1 hours", value)
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: root.hoursIntervalValue != root.hoursIntervalValueDefault
				}
			}

			SpinBox {
				id: minutesInterval
				value: root.minutesIntervalValue
				from: 0
				to: 60
				editable: true
				onValueChanged: cfg_RefreshTime = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

				textFromValue: function(value, locale) {
					return i18ndp("homm3mapwallpaper","%1 minute", "%1 minutes", value)
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: root.minutesIntervalValue != root.minutesIntervalValueDefault
				}
			}

			SpinBox {
				id: secondsInterval
				value: root.secondsIntervalValue
				from: 0
				to: 60
				editable: true
				onValueChanged: cfg_RefreshTime = hoursInterval.value * 3600 + minutesInterval.value * 60 + secondsInterval.value

				textFromValue: function(value, locale) {
					return i18ndp("homm3mapwallpaper","%1 second", "%1 seconds", value)
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: root.secondsIntervalValue != root.secondsIntervalValueDefault
				}
			}
		}

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "Displayed map level:")

			ComboBox {
				id: displayedMapLevelCombobox

				model: [
					{
						'label': i18nd("homm3mapwallpaper", "Random"),
						'displayedLevel': -1
					},
					{
						'label': i18nd("homm3mapwallpaper", "Surface"),
						'displayedLevel': 0
					},
					{
						'label': i18nd("homm3mapwallpaper", "Underground"),
						'displayedLevel': 1
					}
				]
				textRole: "label"
				onCurrentIndexChanged: {
					cfg_DisplayedMapLevel = model[currentIndex]["displayedLevel"];
				}
				Component.onCompleted: {
					slideshowModeComboBox.currentIndex = cfg_DisplayedMapLevel + 1;
				}

				KCM.SettingHighlighter {
					highlight: cfg_DisplayedMapLevel != -1
				}
			}
		}

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "Initial displayed position (in game tiles):")

			CheckBox {
				id: randomInitialPositionCheckBox
				text: i18nd("homm3mapwallpaper", "Random position")
				checked: cfg_RandomInitialPosition
				onToggled: root.cfg_RandomInitialPosition = randomInitialPositionCheckBox.checked

				KCM.SettingHighlighter {
					highlight: cfg_RandomInitialPosition !== cfg_RandomInitialPositionDefault
				}
			}

			SpinBox {
				id: positionXbox
				value: root.cfg_InitialPositionX
				from: -1
				to: 145
				enabled: cfg_RandomInitialPosition === false
				editable: true
				onValueChanged: cfg_InitialPositionX = positionXbox.value

				textFromValue: function(value, locale) {
					return value;
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: cfg_InitialPositionX != cfg_InitialPositionXDefault
				}
			}

			SpinBox {
				id: positionYbox
				value: root.cfg_InitialPositionY
				from: -1
				to: 145
				enabled: cfg_RandomInitialPosition === false
				editable: true
				onValueChanged: cfg_InitialPositionY = positionYbox.value

				textFromValue: function(value, locale) {
					return value;
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: cfg_InitialPositionY != cfg_InitialPositionYDefault
				}
			}
		}

		RowLayout {
			Kirigami.FormData.label: i18nd("homm3mapwallpaper", "Map scaling factor, percentage:")

			SpinBox {
				id: mapScalingFactorBox
				from: 1
				value: root.cfg_Scale * 100
				to: 100 * 100
				stepSize: 10
				editable: true

				onValueChanged: cfg_Scale = mapScalingFactorBox.value / 100

				textFromValue: function(value, locale) {
					return value;
				}
				valueFromText: function(text, locale) {
					return parseInt(text);
				}

				KCM.SettingHighlighter {
					highlight: cfg_Scale != cfg_ScaleDefault
				}
			}
		}
	}
}
