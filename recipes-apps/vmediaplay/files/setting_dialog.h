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
#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialog>

class Setting_Dialog : public QDialog
{
	Q_OBJECT

public:
	explicit Setting_Dialog(QWidget *parent = nullptr);
	~Setting_Dialog();
	
	QWidget *widget;
	QWidget *centralwidget;
	QVBoxLayout *main_layout;
	QGridLayout *option_layout;
	QHBoxLayout *button_layout;

	QLabel *label;
	QComboBox *comboBox;
	QLabel *label_2;
	QComboBox *comboBox_2;
	QLabel *label_3;
	QComboBox *comboBox_3;
	QLabel *label_4;
	QComboBox *comboBox_4;
	QLabel *label_5;
	QPushButton *pushButton;

private:
	void do_update();	

};

#endif