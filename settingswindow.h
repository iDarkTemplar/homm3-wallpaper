/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QStringList>
#include <QtCore/QStringListModel>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow: public QWidget
{
	Q_OBJECT

public:
	explicit SettingsWindow(QWidget *parent = nullptr);
	~SettingsWindow();

	QStringList getItems();
	void setItems(const QStringList &items);

	void openArchivesDialog();
	void removeSelectedEntries();

	void dialogAccepted();
	void dialogRejected();

signals:
	void accepted();

private:
	Ui::SettingsWindow *ui;

	QStringListModel m_model;
};
