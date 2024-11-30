/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtWidgets/QFileDialog>

#include "homm3_image_provider.h"
#include "homm3map.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->quickWidget->engine()->addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider);
	ui->quickWidget->setSource(QStringLiteral("qrc:/main.qml"));

	QObject::connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openMapDialog);
	QObject::connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
	QObject::connect(ui->actionToggle_level, &QAction::triggered, this, &MainWindow::toggleMapLevel);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openMapDialog()
{
	QString map_name = QFileDialog::getOpenFileName(this, QObject::tr("Open map file"), QString(), QObject::tr("Map files (*.h3m)"));

	if (map_name.isEmpty())
	{
		return;
	}

	auto map = getMapObject();
	if (map == nullptr)
	{
		return;
	}

	map->loadMap(map_name, 0);
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

void MainWindow::setMap(const QString &name)
{
	auto map = getMapObject();
	if (map == nullptr)
	{
		return;
	}

	map->loadMap(name, 0);
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
