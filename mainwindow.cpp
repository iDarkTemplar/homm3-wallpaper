/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QTimer>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include "homm3_image_provider.h"
#include "homm3map.h"
#include "homm3singleton.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_settings(QStringLiteral("homm3map-viewer"))
{
	ui->setupUi(this);

	auto data_archives = m_settings.value(QStringLiteral("Archives"), QStringList()).toStringList();

	Homm3MapSingleton::getInstance()->setDataArchives(data_archives);

	m_settings_window.setWindowModality(Qt::ApplicationModal);

	ui->quickWidget->engine()->addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider);
	ui->quickWidget->setSource(QStringLiteral("qrc:/main.qml"));

	auto *map = getMapObject();
	if (map != nullptr)
	{
		QObject::connect(map, &Homm3Map::loadingFinished, this, &MainWindow::setLastMap);
	}

	{
		auto map_name = m_settings.value(QStringLiteral("LastMap"), QString()).toString();
		if (!map_name.isEmpty())
		{
			setMap(map_name, m_settings.value(QStringLiteral("LastLevel"), 0).toInt());
		}
	}

	QObject::connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openMapDialog);
	QObject::connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettingsWindow);
	QObject::connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	QObject::connect(ui->actionToggle_level, &QAction::triggered, this, &MainWindow::toggleMapLevel);

	QObject::connect(&m_settings_window, &SettingsWindow::accepted, this, &MainWindow::settingsUpdated);

	if (data_archives.isEmpty())
	{
		QTimer::singleShot(0,[](){
			QMessageBox::warning(
				nullptr,
				QObject::tr("Configuration warning"),
				QObject::tr("No HOMM3 data archives are set.\nGo to menu 'File' -> 'Settings' and add required data archives.\nAfter that a map may be opened via 'File' -> 'Open' menu."));
		});
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openMapDialog()
{
	QString map_name = QFileDialog::getOpenFileName(this, QObject::tr("Open map file"), QString(), QObject::tr("Map file (*.h3m)"));
	if (map_name.isEmpty())
	{
		return;
	}

	setMap(map_name, 0);
}

void MainWindow::openSettingsWindow()
{
	if (m_settings_window.isVisible())
	{
		return;
	}

	QStringList values = m_settings.value(QStringLiteral("Archives"), QStringList()).toStringList();

	m_settings_window.setItems(values);
	m_settings_window.show();
}

Homm3Map* MainWindow::getMapObject() const
{
	auto root_object = ui->quickWidget->rootObject();
	if (root_object == nullptr)
	{
		return nullptr;
	}

	auto map = root_object->findChild<Homm3Map*>("map");

	return map;
}

void MainWindow::setMap(const QString &name, int level)
{
	auto map = getMapObject();
	if (map == nullptr)
	{
		return;
	}

	map->loadMap(name, level);
}

void MainWindow::toggleMapLevel()
{
	auto map = getMapObject();
	if (map == nullptr)
	{
		return;
	}

	map->toggleLevel();
}

void MainWindow::settingsUpdated()
{
	QStringList values = m_settings_window.getItems();

	m_settings.setValue(QStringLiteral("Archives"), values);

	// reload main view
	Homm3MapSingleton::getInstance()->setDataArchives(values);

	auto map = getMapObject();
	QString map_name = map->currentMapName();
	int level = map->mapLevel();

	ui->quickWidget->setSource(QStringLiteral("qrc:/main.qml"));

	map = getMapObject();
	if (map != nullptr)
	{
		QObject::connect(map, &Homm3Map::loadingFinished, this, &MainWindow::setLastMap);

		if (!map_name.isEmpty())
		{
			map->loadMap(map_name, level);
		}
	}
}

void MainWindow::setLastMap(QString name, int level)
{
	m_settings.setValue(QStringLiteral("LastMap"), name);
	m_settings.setValue(QStringLiteral("LastLevel"), level);
}
