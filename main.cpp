/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022-2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include <stdio.h>
#include <inttypes.h>

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <QtWidgets/QApplication>
#include <QtQml/QQmlApplicationEngine>

#include "def_file.h"
#include "homm3map.h"
#include "homm3singleton.h"
#include "lod_archive.h"

#include "vcmi/CBinaryReader.h"
#include "vcmi/CFileInputStream.h"

#include "mainwindow.h"

int main(int argc, char **argv)
{
	try
	{
		if (argc < 3)
		{
			fprintf(stderr,
				"ERROR: not enough arguments\n"
				"\n"
				"USAGE: %s map archive [archive ...]\n"
				"\twhere archive is usually H3sprite.lod and H3ab_spr.lod\n",
				argv[0]);

			return -1;
		}

		{
			QStringList files_list;

			for (int i = 2; i < argc; ++i)
			{
				files_list.push_back(QString::fromLocal8Bit(argv[i]));
			}

			Homm3MapSingleton::getInstance()->setDataArchives(files_list);
		}

		QApplication app(argc, argv);

		qmlRegisterType<Homm3Map>("homm3mapprivate", 1, 0, "Homm3Map");

		MainWindow mainWindow;

		mainWindow.setMap(QString::fromLocal8Bit(argv[1]));

		mainWindow.show();

		return app.exec();
	}
	catch (const std::exception &e)
	{
		printf("Caught exception: %s\n", e.what());
	}

	return -1;
}
