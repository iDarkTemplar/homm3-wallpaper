/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include <stdio.h>

#include <stdexcept>

#include <QtWidgets/QApplication>

#include "homm3map.h"

#include "mainwindow.h"

int main(int argc, char **argv)
{
	try
	{
		QApplication app(argc, argv);

		qmlRegisterType<Homm3Map>("homm3mapprivate", 1, 0, "Homm3Map");

		MainWindow mainWindow;

		mainWindow.show();

		return app.exec();
	}
	catch (const std::exception &e)
	{
		printf("Caught exception: %s\n", e.what());
	}

	return -1;
}
