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

#include "wifi.h"
#include "ui_mainwindow.h"

using namespace std;

struct wifi_info_s {
	int in_use;
	string bssid;
	string ssid;
	string mode;
	string chan;
	string rate;
	string signal;
	string bars;
	string security;
};

std::list<wifi_info_s> wifi_info_list;
QTimer *wifi_timer;

extern string exec_cmd(string command);

/* =============================================================== */
/* Wi-Fi Dialog */
/* =============================================================== */
WiFi_Dialog::WiFi_Dialog(QWidget *parent) : QDialog(parent),ui(new Ui::WiFi_Dialog)
{
	ui->setupUi(this);
	setWindowModality(Qt::ApplicationModal);
	connect(ui->pushButton1, SIGNAL(clicked()), this, SLOT(btn1_onBtnClick()));
	connect(ui->pushButton2, SIGNAL(clicked()), this, SLOT(btn2_onBtnClick()));
	connect(ui->pushButton3, SIGNAL(clicked()), this, SLOT(btn3_onBtnClick()));
}

WiFi_Dialog::~WiFi_Dialog()
{
	delete ui;
}

/* Delete button */
void WiFi_Dialog::btn1_onBtnClick()
{
	QString str = "nmcli connect delete id " + device_name;
	string result = exec_cmd(str.toStdString());
	QDialog::close();
}

/* Connect button */
void WiFi_Dialog::btn2_onBtnClick()
{
	QString str = ui->label->text();
	QString passwd = ui->lineEdit->text();

	if (passwd.toStdString().length() == 0) {
		passwd = "\"\"";
	}

	str = "nmcli device wifi connect " + device_name + " ifname wlp1s0 password " + passwd;
	string result = exec_cmd(str.toStdString());
	std::size_t pos = result.find("Error");

	QMessageBox msgBox;
	msgBox.setText((pos == std::string::npos) ? "Connect OK" : "Connect NG");
	msgBox.exec();
	QDialog::close();
}

/* Cancel button */
void WiFi_Dialog::btn3_onBtnClick()
{
	QDialog::close();
}

void WiFi_Dialog::set_wifi_label(char *str, int level, int in_use)
{
	char sbuf[100];
	QString qstr;
	string passwd = "";

	device_name = str;

	/* device name label */
	sprintf(sbuf, "/etc/vsettings/wifi%d.png", level);
	QPixmap pixmap(sbuf);
	ui->label1->setPixmap(pixmap);
	ui->label1->setFixedSize(20,20);
	ui->label1->setScaledContents(true);
	ui->label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	sprintf(sbuf, "%s\n (%s)", str, (in_use) ? "connect" : "disconnet");
	ui->label->setText(QApplication::translate("Dialog", sbuf, nullptr));

	/* password */
	qstr = "/etc/NetworkManager/system-connections/" + device_name + ".nmconnection";
	if (QFileInfo::exists(qstr)) {
		qstr = "cat " + qstr;
		string result = exec_cmd(qstr.toStdString());
		size_t pos = result.find("psk=", 0);
		if (pos != std::string::npos) {
			size_t end = result.find("\n", pos);
			passwd = result.substr(pos + 4, end - pos - 4);
		}
	} else {
		ui->pushButton1->setEnabled(0);
	}
	ui->lineEdit->setText(QApplication::translate("Dialog", passwd.c_str(), nullptr));
}

/* =============================================================== */
/* Wi-Fi Class */
/* =============================================================== */
void wifi_get_list(void)
{
	struct wifi_info_s w;
	string str, line;
	size_t pos = 0, end;

	str = exec_cmd("nmcli device wifi list");

	// IN-USE  BSSID SSID MODE CHAN RATE SIGNAL BARS SECURITY
	size_t pos_bssid = str.find("BSSID", 0);
	size_t pos_ssid = str.find("SSID", pos_bssid + 5);
	size_t pos_mode = str.find("MODE", 0);
	size_t pos_chan = str.find("CHAN", 0);
	size_t pos_rate = str.find("RATE", 0);
	size_t pos_signal = str.find("SIGNAL", 0);
	size_t pos_bars = str.find("BARS", 0);
	size_t pos_security = str.find("SECURITY", 0);

	wifi_info_list.clear();
	while(pos < str.length()) {
		pos = str.find("\n", pos) + 1;
		if (pos >= str.length())
			break;
		end = str.find("\n", pos);
		line = str.substr(pos, (end == std::string::npos ? std::string::npos : (end - pos + 1)));

		w.in_use = (str[pos] == '*') ? 1 : 0;
		w.bssid = str.substr(pos + pos_bssid, pos_ssid - pos_bssid);
		w.bssid.erase(w.bssid.find_last_not_of(" ") + 1);
		w.ssid = str.substr(pos + pos_ssid, pos_mode - pos_ssid);
		w.ssid.erase(w.ssid.find_last_not_of(" ") + 1);
		w.mode = str.substr(pos + pos_mode, pos_chan - pos_mode);
		w.mode.erase(w.mode.find_last_not_of(" ") + 1);
		w.chan = str.substr(pos + pos_chan, pos_rate - pos_chan);
		w.chan.erase(w.chan.find_last_not_of(" ") + 1);
		w.rate = str.substr(pos + pos_rate, pos_signal - pos_rate);
		w.rate.erase(w.rate.find_last_not_of(" ") + 1);
		w.signal = str.substr(pos + pos_signal, pos_bars - pos_signal);
		w.signal.erase(w.signal.find_last_not_of(" ") + 1);
		w.bars = str.substr(pos + pos_bars, pos_security - pos_bars);
		w.bars.erase(w.bars.find_last_not_of(" ") + 1);
		w.security = str.substr(pos + pos_security, (end == std::string::npos ? std::string::npos : (end - (pos + pos_security) + 1)));
		w.security.erase(w.security.find_last_not_of(" ") + 1);

		//qDebug("-- %s\n", line.c_str());
		//qDebug("in_use %d,%s,%s,%s,%s,%s,%s,%s,%s\n", w.in_use, w.bssid.c_str(), w.ssid.c_str(), w.mode.c_str(),
		//	w.chan.c_str(), w.rate.c_str(), w.signal.c_str(), w.bars.c_str(), w.security.c_str());
		wifi_info_list.push_back(w);
	}
}

void wifi_enable(int enable)
{
	string cmd = "ifconfig wlp1s0 ";

	cmd.append((enable) ? "up" : "down");
	exec_cmd(cmd);
}

int wifi_is_enable(void)
{
	string str;

	str = exec_cmd("ifconfig wlp1s0");
	if (str.find("UP") == std::string::npos) {
		return 0;
	}
	return 1;
}

void WifiClass::do_wifi_enable(int enable)
{
	wifi_enable(enable);
	if (enable) {
		wifi_timer->start();
	} else {
		wifi_timer->stop();
		wifi_info_list.clear();
		wifi_show_list();
	}
}

void WifiClass::page_enable(int enable)
{
	if (!wifi_is_enable())
		enable = 0;

	if (enable)
		wifi_timer->start();
	else
		wifi_timer->stop();
}

void WifiClass::itemDoubleClicked(QListWidgetItem *item)
{
	char *str;
	int in_use = 0, level = 0;

	wifi_timer->stop();
	WiFi_Dialog *subwindow = new WiFi_Dialog(this);
	str = (char *) item->text().toStdString().c_str();

	for (std::list<wifi_info_s>::iterator it = wifi_info_list.begin(); it != wifi_info_list.end(); it++) {
		if (it->ssid.compare(str) == 0) {
			in_use = it->in_use;
			level = it->bars.length();
			break;
		}
	}
	subwindow->set_wifi_label(str, level, in_use);

	int ref = subwindow->exec();
	wifi_timer->start();
}

void WifiClass::wifi_show_list(void)
{
	wifi_listWidget->clear();
	wifi_listwidget_item_cnt = 0;
	for (std::list<struct wifi_info_s>::iterator it = wifi_info_list.begin(); it != wifi_info_list.end(); it++) {
		char sbuf[100];
		sprintf(sbuf, "/etc/vsettings/wifi%d.png", it->bars.length());

		QListWidgetItem *item = new QListWidgetItem();
		item->setIcon(QIcon(sbuf));
		if (it->in_use) {
			wifi_listWidget->insertItem(0, item);
			// wifi_listWidget->setCurrentItem(item);
		    item->setForeground(QColorConstants::Red);
			item->setBackground(QColorConstants::Yellow);
		} else {
			wifi_listWidget->addItem(item);
		}
		item->setText(QApplication::translate("MainWindow", it->ssid.c_str(), nullptr));
		wifi_listwidget_item_cnt++;
	}
}

void WifiClass::do_wifi_timer(void)
{
	wifi_get_list();
	wifi_show_list();
}

WifiClass::WifiClass(Ui::MainWindow *ui)
{
	new QListWidgetItem(ui->listWidget);
	QListWidgetItem *___qlistwidgetitem = ui->listWidget->item(ui->listwidget_item_cnt);
	___qlistwidgetitem->setText(QApplication::translate("MainWindow", "Wi-Fi", nullptr));
	ui->listwidget_item_cnt++;

	page_wifi = new QWidget();
	page_wifi->setObjectName(QStringLiteral("page_wifi"));
	ui->stackedWidget->addWidget(page_wifi);

	switch_wifi = new Switch(page_wifi);
	switch_wifi->setGeometry(QRect(20, 0, 500, 30));
	switch_wifi->setLayoutDirection(Qt::RightToLeft);
	switch_wifi->setChecked(true);
	// switch_wifi->setDisabled(true);
	switch_wifi->setText(QApplication::translate("MainWindow", "Wi-Fi", nullptr));
	switch_wifi->setChecked(wifi_is_enable());

	wifi_listWidget = new QListWidget(page_wifi);
	wifi_listWidget->setObjectName(QStringLiteral("wifi_listWidget"));
	wifi_listWidget->setGeometry(QRect(20, 50, 500, 300));

	wifi_listwidget_item_cnt = 0;
	wifi_timer = new QTimer();
	wifi_timer->setInterval(5000);

	connect(wifi_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
	QObject::connect(wifi_timer,SIGNAL(timeout()),this,SLOT(do_wifi_timer()));
	QObject::connect(switch_wifi, SIGNAL(stateChanged(int)), this, SLOT(do_wifi_enable(int)));
}

WifiClass::~WifiClass()
{

}
