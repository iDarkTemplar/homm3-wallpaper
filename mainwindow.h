/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCore/QString>
#include <QtCore/QSettings>

#include "settingswindow.h"

namespace Ui {
class MainWindow;
}

class Homm3Map;

class MainWindow: public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openMapDialog();
	void openSettingsWindow();

	void setMap(const QString &name, int level = 0);
	void toggleMapLevel();

	void settingsUpdated();

	void setLastMap(QString name, int level);

private:
	Ui::MainWindow *ui;

	QSettings m_settings;
	SettingsWindow m_settings_window;

	Homm3Map* getMapObject() const;
};
