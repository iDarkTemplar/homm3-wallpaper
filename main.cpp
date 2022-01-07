/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
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
#include "lod_archive.h"
#include "map_object.h"

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

		std::shared_ptr<std::map<std::string, std::tuple<std::string, LodEntry> > > lod_entries = std::make_shared<std::map<std::string, std::tuple<std::string, LodEntry> > >();

		for (int i = 2; i < argc; ++i)
		{
			CFileInputStream file_stream(std::filesystem::path(argv[i]));
			CBinaryReader reader(&file_stream);
			std::vector<LodEntry> files = read_lod_archive_header(reader);

			for (auto iter = files.begin(); iter != files.end(); ++iter)
			{
				(*lod_entries)[iter->name] = std::tie(argv[i], *iter);
			}
		}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

		Homm3MapSingleton::getInstance()->lod_entries = lod_entries;

		QGuiApplication app(argc, argv);

		qmlRegisterType<Homm3Map>("homm3map", 1, 0, "Homm3Map");

		QQmlApplicationEngine engine;

		engine.addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider(lod_entries));

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
