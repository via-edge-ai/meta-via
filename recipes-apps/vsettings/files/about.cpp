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

#include "about.h"
#include "ui_mainwindow.h"

using namespace std;

extern string exec_cmd(string command);

AboutClass::AboutClass(Ui::MainWindow *ui)
{
	new QListWidgetItem(ui->listWidget);
	QListWidgetItem *___qlistwidgetitem3 = ui->listWidget->item(ui->listwidget_item_cnt);
	___qlistwidgetitem3->setText(QApplication::translate("MainWindow", "About", nullptr));
	ui->listwidget_item_cnt++;

	page_about = new QWidget();
	page_about->setObjectName(QStringLiteral("page_about"));
	ui->stackedWidget->addWidget(page_about);

	label_about = new QLabel(page_about);
	label_about->setObjectName(QStringLiteral("label_about"));
	label_about->setGeometry(QRect(30, 30, 500, 200));

	string str = exec_cmd("cat /etc/os-release");
	str.append("\n");
	str.append(exec_cmd("cat /etc/via-release"));
	label_about->setText(QApplication::translate("MainWindow", str.c_str(), nullptr));
}

AboutClass::~AboutClass()
{

}