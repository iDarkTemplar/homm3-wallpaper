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

	void setMap(const QString &name);
	void toggleMapLevel();

private:
	Ui::MainWindow *ui;

	Homm3Map* getMapObject() const;
};
