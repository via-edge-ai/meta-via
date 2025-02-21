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

#include "bt.h"
#include "ui_mainwindow.h"

using namespace std;

struct bt_info_s {
	string name;
	string id;
	int paired;
	int connected;
};

struct bt_var_s {
	std::list<bt_info_s> info_list;
	QTimer *list_timer;
} g_bt;

extern string exec_cmd(string command);

int bt_is_enable(void)
{
	string str;

	str = exec_cmd("hciconfig");
	if (str.find("UP RUNNING") == std::string::npos) {
		return 0;
	}
	return 1;
}

int bt_enable(int enable)
{
	string str = "bluetoothctl power ";

	str.append((enable) ? "on" : "off");
	// qDebug("%s\n", str.c_str());
	str = exec_cmd(str);
	// qDebug("%s\n", str.c_str());
	if (str.find("succeeded") == std::string::npos) {
		return -1;
	}
	return 0;
}

int bt_pair(string id, int enable)
{
	string str = "bluetoothctl ";

	str.append((enable) ? "pair" : "remove");
	str.append(" ");
	str.append(id);
	// qDebug("%s\n", str.c_str());
	str = exec_cmd(str);
	// qDebug("%s\n", str.c_str());
	str = (enable) ? "successful" : "removed";
	if (str.find(str) == std::string::npos) {
		return -1;
	}
	return 0;
}

int bt_connect(string id, int enable)
{
	string str = "bluetoothctl ";

	str.append((enable) ? "connect" : "disconnect");
	str.append(" ");
	str.append(id);
	// qDebug("%s\n", str.c_str());
	str = exec_cmd(str);
	// qDebug("%s\n", str.c_str());
	/* connect -- successful, disconnect -- Successful */
	if (str.find("uccessful") == std::string::npos) {
		return -1;
	}
	return 0;
}

void bt_scan(int sec)
{
	string cmd = "bluetoothctl ";

	if (sec) {
		cmd.append("--timeout ");
		cmd.append(std::to_string(sec));
		cmd.append(" ");
	}
	cmd.append("scan on");
	// qDebug("%s\n", cmd.c_str());
	cmd = exec_cmd(cmd);
}

void bt_get_list(void)
{
	struct bt_info_s info;
	string str, pair_str, connect_str;
	size_t pos = 0, end = 0;
	char *p1, *p2;

	do {
		str = "bluetoothctl devices Paired";
		pair_str = exec_cmd(str);
		qDebug("pair cmd: %s\n", pair_str.c_str());
		if (pair_str.find("] Device") == std::string::npos) /* skip scan log */
			break;
	} while(1);

	do {
		str = "bluetoothctl devices Connected";
		connect_str = exec_cmd(str);
		qDebug("conn cmd: %s\n", connect_str.c_str());
		if (connect_str.find("] Device") == std::string::npos) /* skip scan log */
			break;
	} while(1);

	str = "bluetoothctl devices";
	str = exec_cmd(str);
	g_bt.info_list.clear();
	while(pos < str.length()) {
		if(strncmp(str.data() + pos, "Device", 6) != 0) {
			pos = str.find("\n", pos) + 1;
			continue;
		}
		info.id = str.substr(pos + 7, 17);
		pos = pos + 25;
		end = str.find("\n", pos);
		info.name = str.substr(pos, end - pos);
		pos = end + 1;
		p1 = info.id.data();
		p2 = info.name.data();
		if ((p1[0] == p2[0]) && (p1[1] == p2[1]) && (p2[2] == '-'))
			continue;
		info.paired = (pair_str.find(info.id.c_str()) == std::string::npos) ? 0 : 1;
		info.connected = (connect_str.find(info.id.c_str()) == std::string::npos) ? 0 : 1;
		qDebug("++ id %s,name %s,pair %d,connect %d", info.id.c_str(), info.name.c_str(), info.paired, info.connected);
		g_bt.info_list.push_back(info);
	}
}

void BTClass::bt_show_list(void)
{
	int pair_cnt = 0, connect_cnt = 0, index;

	bt_listWidget->clear();
	for (std::list<struct bt_info_s>::iterator it = g_bt.info_list.begin(); it != g_bt.info_list.end(); it++) {
		QListWidgetItem *item = new QListWidgetItem();
		item->setText(QApplication::translate("MainWindow", it->name.c_str(), nullptr));

		index = 0;
		if (it->connected) {
			item->setBackground(QColorConstants::Yellow);
			connect_cnt++;
			index = connect_cnt;
		}

		if (it->paired) {
			item->setForeground(QColorConstants::Red);
			if (!it->connected) {
				pair_cnt++;
				index += pair_cnt;
			}
		}

		if (index)
			bt_listWidget->insertItem(index - 1, item);
		else
			bt_listWidget->addItem(item);
	}
}

void BTClass::do_list_timer(void)
{
	bt_get_list();
	bt_show_list();
}

void BTClass::bt_switch_enable(int enable)
{
	bt_enable(enable);
	if (enable) {
		scan_thread->start();
		g_bt.list_timer->start();
	} else {
		scan_thread->terminate();
		g_bt.list_timer->stop();
		g_bt.info_list.clear();
		bt_show_list();
	}
}

void BTClass::page_enable(int enable)
{
	if (!bt_is_enable())
		enable = 0;

	if (enable) {
		g_bt.list_timer->start();
		if (!scan_thread->isRunning())
			scan_thread->start();
	} else {
		g_bt.list_timer->stop();
		if (scan_thread->isRunning())
			scan_thread->terminate();
	}
}

void BTClass::itemDoubleClicked(QListWidgetItem *item)
{
	struct bt_info_s *info;
	int ret = 0;

	g_bt.list_timer->stop();

	for (std::list<bt_info_s>::iterator it = g_bt.info_list.begin(); it != g_bt.info_list.end(); it++) {
		if (it->name.compare(bt_listWidget->currentItem()->text().toStdString().c_str()) == 0) {
			info = &*it;
			break;
		}
	}

	QMessageBox msgbox;
	msgbox.setWindowTitle("");
	msgbox.setText(QString::fromStdString(info->name));
	msgbox.setInformativeText(QString::fromStdString(info->id));

	QPushButton *btn_ignore = 0, *btn_delete = 0, *btn_disconnect = 0, *btn_connect = 0, *btn_pair = 0;
	if (info->connected) {
		btn_delete = msgbox.addButton(tr("Delete"), QMessageBox::ActionRole);
		btn_disconnect = msgbox.addButton(tr("Disconnect"), QMessageBox::ActionRole);
		btn_ignore = msgbox.addButton(QMessageBox::Ignore);
	} else if (info->paired) {
		btn_delete = msgbox.addButton(tr("Delete"), QMessageBox::ActionRole);
		btn_connect = msgbox.addButton(tr("Connect"), QMessageBox::ActionRole);
		btn_ignore = msgbox.addButton(QMessageBox::Ignore);
	} else {
		btn_pair = msgbox.addButton(tr("Pair"), QMessageBox::ActionRole);
		btn_connect = msgbox.addButton(tr("Connect"), QMessageBox::ActionRole);
		btn_ignore = msgbox.addButton(QMessageBox::Ignore);
	}
	msgbox.setDefaultButton(QMessageBox::Ignore);
	msgbox.exec();
	if (msgbox.clickedButton() == btn_pair) {
		ret = bt_pair(info->id, 1);
		if (ret == 0)
			ret = bt_connect(info->id, 1);
	}
	if (msgbox.clickedButton() == btn_delete) {
		ret = bt_pair(info->id, 0);
	}
	if (msgbox.clickedButton() == btn_connect) {
		ret = bt_connect(info->id, 1);
	}
	if (msgbox.clickedButton() == btn_disconnect) {
		ret = bt_connect(info->id, 0);
	}

	if (msgbox.clickedButton() != btn_ignore) {
		string str = (ret) ? "Command Fail" : "Command OK";
		QMessageBox msgbox2;
		msgbox2.setWindowTitle("");
		msgbox2.setText(QString::fromStdString(str));
		msgbox2.setDefaultButton(QMessageBox::Ignore);
		msgbox2.exec();
	}

	g_bt.list_timer->start();
}

BTClass::BTClass(Ui::MainWindow *ui)
{
	new QListWidgetItem(ui->listWidget);
	QListWidgetItem *___qlistwidgetitem1 = ui->listWidget->item(ui->listwidget_item_cnt);
	___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "BT", nullptr));
	ui->listwidget_item_cnt++;

	bt_page = new QWidget();
	bt_page->setObjectName(QStringLiteral("page_bt"));
	ui->stackedWidget->addWidget(bt_page);

	bt_switch = new Switch(bt_page);
	bt_switch->setGeometry(QRect(20, 0, 500, 30));
	bt_switch->setLayoutDirection(Qt::RightToLeft);
	bt_switch->setChecked(true);
	bt_switch->setText(QApplication::translate("MainWindow", "BT", nullptr));
	bt_switch->setChecked(bt_is_enable());
	connect(bt_switch, SIGNAL(stateChanged(int)), this, SLOT(bt_switch_enable(int)));

	bt_listWidget = new QListWidget(bt_page);
	bt_listWidget->setObjectName(QStringLiteral("bt_listWidget"));
	bt_listWidget->setGeometry(QRect(20, 50, 500, 300));
	connect(bt_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));

	scan_thread = new BT_Scan;
	g_bt.list_timer = new QTimer();
	g_bt.list_timer->setInterval(3000);
	QObject::connect(g_bt.list_timer,SIGNAL(timeout()),this,SLOT(do_list_timer()));
}

BTClass::~BTClass()
{

}

BT_Scan::BT_Scan() {

}

void BT_Scan::run()
{
	while(1) {
		bt_scan(0);
		msleep(1000);
	}
}
