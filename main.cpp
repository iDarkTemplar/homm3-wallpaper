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
#include <stdexcept>
#include <tuple>

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include "def_file.h"
#include "homm3_image_provider.h"
#include "lod_archive.h"

#include "vcmi/CBinaryReader.h"
#include "vcmi/CCompressedStream.h"
#include "vcmi/CFileInputStream.h"

int main(int argc, char **argv)
{
	try
	{
		if (argc < 2)
		{
			fprintf(stderr, "ERROR: not enough arguments\n");
			return -1;
		}

		std::map<std::string, std::tuple<std::string, LodEntry> > lod_entries;

		for (int i = 1; i < argc; ++i)
		{
			CFileInputStream file_stream(std::filesystem::path(argv[i]));
			CBinaryReader reader(&file_stream);
			std::vector<LodEntry> files = read_lod_archive_header(reader);

			for (auto iter = files.begin(); iter != files.end(); ++iter)
			{
				lod_entries[iter->name] = std::tie(argv[i], *iter);
			}
		}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

		QGuiApplication app(argc, argv);

		QQmlApplicationEngine engine;
		const QUrl url(QStringLiteral("qrc:/main.qml"));

		engine.addImageProvider(QStringLiteral("homm3"), new Homm3ImageProvider(lod_entries));

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
