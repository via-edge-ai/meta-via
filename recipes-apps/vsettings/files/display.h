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
#ifndef DISPLAY_H
#define DISPLAY_H

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSlider>
#include <QVBoxLayout>
#include <QFileInfo>

namespace Ui {
class MainWindow;
}

class DisplayClass : public QMainWindow
{
	Q_OBJECT

public:
	explicit DisplayClass(Ui::MainWindow *ui);
	~DisplayClass();
	QWidget *page_disp;
	QVBoxLayout *backlight_layout;
	QSlider *slider_dsi;
	QSlider *slider_edp;
	QSlider *slider_dp;

public slots:
	void set_dsi_backlight(int value);
	void set_edp_backlight(int value);
	void set_dp_backlight(int value);

private:
	Ui::MainWindow *ui;
};

#endif // DISPLAY_H