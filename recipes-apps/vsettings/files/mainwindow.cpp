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

#include <QStandardItemModel>
#include <QStandardItem>
#include <QFont>
#include <QDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QStandardItemModel *model;

	/* windows ui */
	ui->setupUi(this);
	setWindowModality(Qt::ApplicationModal);

	QFont font("Roboto medium", 13);
	ui->listWidget->setFont(font);

	wifi = new WifiClass(ui);
	bt = new BTClass(ui);
	new DisplayClass(ui);
	new AboutClass(ui);

	InitControls();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::currentRowChanged(int index)
{
	// qDebug("%s(%d)\n", __func__, index);
	ui->stackedWidget->setCurrentIndex(index);
	wifi->page_enable((index == 0) ? 1 : 0);
	bt->page_enable((index == 1) ? 1 : 0);
}

void MainWindow::InitControls()
{
	// connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
	connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChanged(int)));
	ui->stackedWidget->setCurrentIndex(0);
}

string exec_cmd(string command) {
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