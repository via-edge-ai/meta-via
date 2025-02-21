/*
 * Copyright (c) 2024 VIA Technologies, Inc. All Rights Reserved.
 *
 * This PROPRIETARY SOFTWARE is the property of VIA Technologies, Inc.
 * and may contain trade secrets and/or other confidential information of
 * VIA Technologies, Inc. This file shall not be disclosed to any
 * third party, in whole or in part, without prior written consent of VIA.
 *
 * THIS PROPRIETARY SOFTWARE AND ANY RELATED DOCUMENTATION ARE PROVIDED
 * AS IS, WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND EITHER EXPRESS
 * OR IMPLIED, AND VIA TECHNOLOGIES, INC. DISCLAIMS ALL EXPRESS
 * OR IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * QUIET ENJOYMENT OR NON-INFRINGEMENT.
 */
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>

#include <QString>
#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include <QStandardItemModel>
#include <QStandardItem>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

#define APP_NAME "VPlay"

struct global_var_s {
	string platform;
	int is_debian;
	int is_audio;
} g;

string exec(string command) {
	char buffer[128];
	string result = "";

	// Open pipe to file
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		return "popen failed!";
	}

	// read till end of process:
	while (!feof(pipe)) {

		// use buffer to read and add to result
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}

	pclose(pipe);
	return result;
}

int get_platform_name(void)
{
	std::size_t pos,rpos;
	string str = exec("fw_printenv boot_conf");

	pos = str.find("#conf-mediatek_");
	if (pos == std::string::npos) {
		return -1;
	}
	rpos = str.find(".dtb#");
	g.platform = str.substr(pos + 15, rpos - pos - 15);

	str = exec("cat /etc/os-release");
	pos = str.find("ID=debian");
	g.is_debian = (pos == std::string::npos) ? 0 : 1;

	// qDebug("platform:%s,is_debian %d\n", g.platform.c_str(), g.is_debian);
	return 0;
}

int parse_mdedia_info(Ui::MainWindow *ui)
{
	QString file_name;
	string str;
	std::size_t pos;

	str = "gst-discoverer-1.0 ";
	file_name = ui->lineEdit_2->text();
	str.append(file_name.toStdString());
	str = exec(str);
	pos = str.find("audio");
	g.is_audio = (pos == std::string::npos) ? 0 : 1;
	return 0;
}

string do_cmd(Ui::MainWindow *ui) {
	QString file_name;
	string str;

	parse_mdedia_info(ui);

	str = "gst-launch-1.0 -v filesrc location=";
	file_name = ui->lineEdit_2->text();
	str.append(file_name.toStdString());
	str.append(" ! qtdemux name=demux");
	str.append(" demux.video_0 ! queue !");
	switch(ui->comboBox->currentIndex()) {
	case 0:
	default:
		str.append(" decodebin !");
		break;
	case 1:
		str.append(" parsebin ! v4l2h264dec !");
		break;
	case 2:
		str.append(" parsebin ! v4l2h265dec !");
		break;
	case 3:
		str.append(" parsebin ! v4l2mpeg4dec !");
		break;
	}
	str.append(" v4l2convert output-io-mode=dmabuf-import capture-io-mode=dmabuf !");
	switch(ui->comboBox_2->currentIndex()) {
	case 0:
	default:
		str.append(" waylandsink");
		break;
	case 1:
		str.append(" waylandsink fullscreen=true");
		break;
	}

	if (!g.is_audio) // no audio
		goto do_cmd_end;

	str.append(" demux.audio_0 ! queue !");
	str.append(" decodebin !");
	str.append(" audioconvert ! audioresample !");
	if (g.is_debian) { /* debian or ubuntu */
		str.append(" autoaudiosink");
	} else { /* yocto */
		switch(ui->comboBox_3->currentIndex()) {
		case 0:
			str.append(" alsasink device=\"hw:0,5\"");
			break;
		case 1:
			str.append(" alsasink device=\"hw:0,0\"");
			break;
		}
	}

do_cmd_end:
	// str.append("sync=false async=false");
	// str.append("video/x-raw,height=720,width=1920");
	ui->textBrowser->setText(QString::fromStdString(str));
	return str;
}

void MainWindow::resizeEvent(QResizeEvent*)
{
	char chk_buf[200];
	char radio_buf[200];
	QSize size = this->size();

	int w = size.width() / 40;
	int h = size.height() / 20;
	int font_size = (w > h) ? h : w;

	QFont font = ui->centralwidget->font();
	font.setPointSize(font_size);
	ui->centralwidget->setFont(font);
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	string str;	

	if (get_platform_name()) {
		qDebug("*E* could not find platform\n");
		QMainWindow::close();
		return;
	}

	/* windows ui */
	ui->setupUi(this);

	// str = APP_NAME + " ("s + g.platform + ")"s;
	str = APP_NAME;
	MainWindow::setWindowTitle(QApplication::translate("MainWindow", str.c_str(), nullptr));

	ui->lineEdit_2->setText("");
	ui->comboBox->setCurrentIndex(0); // decodebin
	ui->comboBox_2->setCurrentIndex(1); // fullscreen
	ui->comboBox_3->setCurrentIndex(0); // hdmi
	ui->comboBox_4->setCurrentIndex(1); // loop count
	ui->textBrowser->setEnabled(0);
	connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::do_play);
	connect(ui->pushButton_2, &QPushButton::clicked, this, &QMainWindow::close);
	connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::do_copy);
	connect(ui->toolButton, &QToolButton::clicked, this, &MainWindow::do_file);
	connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(do_update()));
	connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(do_update()));
	connect(ui->comboBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT(do_update()));
	connect(ui->comboBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT(do_update()));

	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->comboBox_3->model());
	Q_ASSERT(model != nullptr);
	bool disabled = true;
	QStandardItem *item;
	if (g.is_debian) {
		ui->comboBox_3->setItemText(0, QApplication::translate("MainWindow", "Debian autosink", nullptr));
		ui->comboBox_3->setEnabled(0);

		item = model->item(0);
		item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled
								: item->flags() | Qt::ItemIsEnabled);
		item = model->item(1);
		item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled
								: item->flags() | Qt::ItemIsEnabled);
	}

	QFont font = ui->centralwidget->font();
	font.setPointSize(8);
	ui->centralwidget->setFont(font);

	do_cmd(ui);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::do_play() {
	string str;
	QString qstr;
	int cnt;

	str = do_cmd(ui);
	qstr = ui->comboBox_4->currentText();
	cnt = qstr.toInt();

	do {
		exec(str);
		if (cnt == 1)
			break;
		if (cnt > 1)
			cnt--;
	} while(1);
}

void MainWindow::do_copy() {
	QClipboard *clipboard = QApplication::clipboard();
	string str;

	str = do_cmd(ui);
	clipboard->setText(QString::fromStdString(str));
}

void MainWindow::do_file() {
	// QString filename = QFileDialog::getOpenFileName(this, tr("Open Media"), QDir::currentPath(), tr("media files (*.mp4 *.avi);;All files (*.*)"));
	QStringList filenames;
	QString filename;
	QFileDialog *dlg=new QFileDialog;

	QFont font = ui->centralwidget->font();
	dlg->setFont(font);
	dlg->setWindowTitle("Open Media");

	QFileInfo fi(ui->lineEdit_2->text());
	dlg->setDirectory(fi.absolutePath());

	dlg->setNameFilter("media files (*.mp4 *.avi);;All files (*.*)");
	dlg->setViewMode(QFileDialog::List);
	// dlg->resize(640,400);
	if(dlg->exec()==QDialog::Accepted) {
		filenames = dlg->selectedFiles();
		filename = filenames[0];
	}

	if (filename.size())
		ui->lineEdit_2->setText(filename);
	do_cmd(ui);
}

void MainWindow::do_update() {
	string str;

	str = do_cmd(ui);
}
