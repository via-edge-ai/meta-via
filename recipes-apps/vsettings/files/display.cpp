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

#include "display.h"
#include "ui_mainwindow.h"

using namespace std;

extern string exec_cmd(string command);

#define DRM_DSI "/sys/class/drm/card0-DSI-1"
#define DRM_EDP "/sys/class/drm/card0-EDP-1"
#define DRM_DP "/sys/class/drm/card0-DP-1"
#define DRM_HDMI "/sys/class/drm/card0-HDMI-A-1"

#define BL_DSI "/sys/devices/platform/backlight_lcd0/backlight/backlight_lcd0"
#define BL_EDP "/sys/devices/platform/backlight_lcd0/backlight/backlight_lcd0"
#define BL_DP "/sys/devices/platform/soc/soc:pwmleds/leds/lcd-backlight-dp"

int disp_is_exist(QString str)
{
	if (QFileInfo::exists(str)) {
		return 1;
	}
	return 0;
}

int get_backlight_max(string disp)
{
	bool ret;
	string str = "cat ";
	str.append(disp);
	str.append("/max_brightness");
	str = exec_cmd(str);
	return QString::fromStdString(str).toInt(&ret);
}

int get_backlight(string disp)
{
	bool ret;
	string str = "cat ";
	str.append(disp);
	str.append("/brightness");
	str = exec_cmd(str);
	return QString::fromStdString(str).toInt(&ret);
}

void set_backlight(string disp, int value)
{
	string str = "echo ";
	str.append(std::to_string(value));
	str.append(" > ");
	str.append(disp);
	str.append("/brightness");
	exec_cmd(str);
}

void DisplayClass::set_dsi_backlight(int value)
{
	set_backlight(BL_DSI, value);
}

void DisplayClass::set_edp_backlight(int value)
{
	set_backlight(BL_EDP, value);
}

void DisplayClass::set_dp_backlight(int value)
{
	set_backlight(BL_DP, value);
}

DisplayClass::DisplayClass(Ui::MainWindow *ui)
{
	new QListWidgetItem(ui->listWidget);
	QListWidgetItem *___qlistwidgetitem2 = ui->listWidget->item(ui->listwidget_item_cnt);
	___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "Display", nullptr));
	ui->listwidget_item_cnt++;

	page_disp = new QWidget();
	page_disp->setObjectName(QStringLiteral("page_disp"));
	ui->stackedWidget->addWidget(page_disp);

	backlight_layout = new QVBoxLayout();
	backlight_layout->setMargin(20);
	backlight_layout->setSpacing(20);
	backlight_layout->setAlignment(Qt::AlignTop);
	page_disp->setLayout(backlight_layout);

	if (disp_is_exist(DRM_DSI)) {
		QLabel *label = new QLabel("DSI backlight");
		backlight_layout->addWidget(label);

		slider_dsi = new QSlider(Qt::Horizontal, this);
		backlight_layout->addWidget(slider_dsi);
		slider_dsi->setMaximum(20);
		slider_dsi->setMaximum(get_backlight_max(BL_DSI));
		slider_dsi->setPageStep(1);
		slider_dsi->setValue(get_backlight(BL_DSI));
		slider_dsi->setTracking(true);
		connect(slider_dsi, SIGNAL(valueChanged(int)),this,SLOT(set_dsi_backlight(int)));
	}

	if (disp_is_exist(DRM_EDP)) {
		QLabel *label = new QLabel("EDP backlight");
		backlight_layout->addWidget(label);

		slider_edp = new QSlider(Qt::Horizontal, this);
		backlight_layout->addWidget(slider_edp);
		slider_edp->setMaximum(20);
		slider_edp->setMaximum(get_backlight_max(BL_DP));
		slider_edp->setPageStep(1);
		slider_edp->setValue(get_backlight(BL_DP));
		slider_edp->setTracking(true);
		connect(slider_edp, SIGNAL(valueChanged(int)),this,SLOT(set_edp_backlight(int)));
	}

	if (disp_is_exist(DRM_DP)) {
		QLabel *label = new QLabel("DP backlight");
		backlight_layout->addWidget(label);

		slider_dp = new QSlider(Qt::Horizontal, this);
		backlight_layout->addWidget(slider_dp);
		slider_dp->setMaximum(20);
		slider_dp->setMaximum(get_backlight_max(BL_DP));
		slider_dp->setPageStep(1);
		slider_dp->setValue(get_backlight(BL_DP));
		// slider_dp->setInvertedAppearance(true);
		slider_dp->setTracking(true);
		connect(slider_dp, SIGNAL(valueChanged(int)),this,SLOT(set_dp_backlight(int)));
	}
}

DisplayClass::~DisplayClass()
{

}