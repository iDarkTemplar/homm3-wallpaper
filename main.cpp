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

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

#include "def_file.h"
#include "homm3_image_provider.h"
#include "homm3map.h"
#include "homm3singleton.h"
#include "lod_archive.h"

#include "vcmi/CBinaryReader.h"
#include "vcmi/CFileInputStream.h"

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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

		QGuiApplication app(argc, argv);

		qmlRegisterType<Homm3Map>("homm3mapprivate", 1, 0, "Homm3Map");

		QQmlApplicationEngine engine;

		engine.addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider);

		engine.rootContext()->setContextProperty(QStringLiteral("map_name"), QString::fromLocal8Bit(argv[1]));

		const QUrl url(QStringLiteral("qrc:/main.qml"));

		QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
			if ((!obj) && (url == objUrl)) {
				QCoreApplication::exit(-1);
			}
		}, Qt::QueuedConnection);

		engine.load(url);

		return app.exec();
	}
	catch (const std::exception &e)
	{
		printf("Caught exception: %s\n", e.what());
	}

	return -1;
}
