/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2024 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <QtWidgets/QFileDialog>

SettingsWindow::SettingsWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::SettingsWindow)
{
	ui->setupUi(this);

	QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsWindow::dialogAccepted);
	QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsWindow::dialogRejected);

	QObject::connect(ui->buttonAdd, &QPushButton::clicked, this, &SettingsWindow::openArchivesDialog);
	QObject::connect(ui->buttonRemove, &QPushButton::clicked, this, &SettingsWindow::removeSelectedEntries);
}

SettingsWindow::~SettingsWindow()
{
	delete ui;
}

QStringList SettingsWindow::getItems()
{
	QStringList result;

	for (int i = 0; i < ui->listWidget->count(); ++i)
	{
		auto *item = ui->listWidget->item(i);
		if (item == nullptr)
		{
			continue;
		}

		result.append(item->text());
	}

	return result;
}

void SettingsWindow::setItems(const QStringList &items)
{
	ui->listWidget->clear();

	for (const auto &item: items)
	{
		ui->listWidget->addItem(item);
	}
}

void SettingsWindow::openArchivesDialog()
{
	QStringList archive_names = QFileDialog::getOpenFileNames(this, QObject::tr("Open archive files"), QString(), QObject::tr("Archive files (*.lod)"));
	if (archive_names.isEmpty())
	{
		return;
	}

	for (const auto &item: archive_names)
	{
		ui->listWidget->addItem(item);
	}
}

void SettingsWindow::removeSelectedEntries()
{
	qDeleteAll(ui->listWidget->selectedItems());
}

void SettingsWindow::dialogAccepted()
{
	emit accepted();
	hide();
}

void SettingsWindow::dialogRejected()
{
	hide();
}
