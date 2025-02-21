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
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>
#include <list>

#include <QLabel>
#include <QVBoxLayout>
#include <QDialog>
#include <QScreen>
#include <QFont>
#include <QSignalMapper>
#include <QSizePolicy>
#include <QString>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

#define APP_NAME "VCenter"

struct app_s {
	string name;
	string exec;
};

struct global_var_s {
	int column;
	int row;

	float scale;

	string platform;
	std::list<app_s> apps;
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
	return 0;
}

int get_config(void)
{
	string path_str = "/etc/"s + APP_NAME + "/"s;
	string file_str;

	transform(path_str.begin(), path_str.end(), path_str.begin(), ::tolower);

	file_str = path_str + "conf.d/" + g.platform + ".conf";
	if( std::filesystem::exists(file_str.c_str()) != true ) {
		file_str = path_str + "default.conf";
		if( std::filesystem::exists(file_str.c_str()) != true ) {
			return -1;
		}
	}

	ifstream fjson(file_str.c_str());
	stringstream buffer;
	buffer << fjson.rdbuf();
	auto json = nlohmann::json::parse(buffer.str());

	g.column = json["numOfColumn"];
	g.row = json["numOfRow"];

	for (auto it : json["apps"]) {
		struct app_s app;

		it["name"].get_to(app.name);
		it["exec"].get_to(app.exec);
		// qDebug("name : %s", app.name.c_str());
		// qDebug("exec : %s", app.exec.c_str());
		g.apps.push_back(app);
	}
	return 0;
}

void MainWindow::doClicked(const QString& btnname)
{
	// qDebug("doClick+ %s\n", btnname.toStdString().c_str());
	string exec_str = btnname.toStdString() + " " + g.platform;
	exec(exec_str);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
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
	int btn_no = 0;

	if (get_platform_name()) {
		qDebug("*E* could not find platform\n");
		QMainWindow::close();
		return;
	}

	if (get_config()) {
		qDebug("*E* no config file\n");
		QMainWindow::close();
		return;
	}

	/* windows ui */
	ui->setupUi(this);

	str = APP_NAME + " ("s + g.platform + ")"s;
	MainWindow::setWindowTitle(QApplication::translate("MainWindow", str.c_str(), nullptr));

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
	int height = screenGeometry.height();
	int width = screenGeometry.width();

	QSignalMapper *signalMapper = new QSignalMapper(this);

	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);

	QFont font = ui->centralwidget->font();
	font.setPointSize(25);
	ui->centralwidget->setFont(font);

	for (std::list<struct app_s>::iterator it = g.apps.begin(); it != g.apps.end(); it++) {
		QPushButton *pushButton;
		QString qstr = QString::fromStdString(it->exec);

		pushButton = new QPushButton(qstr);
		pushButton->setObjectName(QStringLiteral("pushButton"));
		sizePolicy1.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
		pushButton->setSizePolicy(sizePolicy1);

		connect(pushButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
		signalMapper->setMapping(pushButton, qstr);

		ui->gridLayout->addWidget(pushButton, btn_no / g.column, btn_no % g.column, 1, 1);
		pushButton->setText(QApplication::translate("MainWindow", it->name.c_str(), nullptr));
		btn_no++;
	}

	connect(signalMapper, SIGNAL(mapped(const QString &)), this, SLOT(doClicked(const QString &)));
}

MainWindow::~MainWindow()
{
	delete ui;
}