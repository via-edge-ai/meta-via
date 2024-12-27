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
#include "setting_dialog.h"

extern int g_decode;	// 0:decodebin,1:v4l2h264dec,2:v4l2h265dec,3:v4l2mpeg4dec
extern int g_display;// 0:original,1:fullscreen
extern int g_audio;	// 0:hdmi,1:headphone
extern int g_loop;	// loop count

Setting_Dialog::Setting_Dialog(QWidget *parent)
{
	if (this->objectName().isEmpty())
		this->setObjectName(QStringLiteral("SettingDialog"));
	this->setWindowModality(Qt::ApplicationModal);
	this->setWindowFlag(Qt::WindowStaysOnTopHint);
	// setWindowFlags(Qt::Tool);
	this->resize(300, 300);
	//MainWindow->setFixedSize(500, 550);
	centralwidget = new QWidget(this);
	centralwidget->setObjectName(QStringLiteral("centralwidget"));
	main_layout = new QVBoxLayout();
	main_layout->setMargin(20);
	main_layout->setSpacing(20);
	centralwidget->setLayout(main_layout);

	option_layout = new QGridLayout();
	button_layout = new QHBoxLayout;
	main_layout->addLayout(option_layout);
	main_layout->addLayout(button_layout);

	/* Codec */
	label = new QLabel("label");
	comboBox = new QComboBox;
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	comboBox->addItem(QString());
	option_layout->addWidget(label, 0, 0);
	option_layout->addWidget(comboBox, 0, 1);

	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
	comboBox->setItemText(0, QApplication::translate("MainWindow", "decodebin", nullptr));
	comboBox->setItemText(1, QApplication::translate("MainWindow", "v4l2h264dec", nullptr));
	comboBox->setItemText(2, QApplication::translate("MainWindow", "v4l2h265dec", nullptr));
	comboBox->setItemText(3, QApplication::translate("MainWindow", "v4l2mpeg4dec", nullptr));

	/* Display */
	label_2 = new QLabel("label2");
	comboBox_2 = new QComboBox;
	comboBox_2->addItem(QString());
	comboBox_2->addItem(QString());
	option_layout->addWidget(label_2, 2, 0);
	option_layout->addWidget(comboBox_2, 2, 1);

	label_2->setText(QApplication::translate("MainWindow", "Display", nullptr));
	comboBox_2->setItemText(0, QApplication::translate("MainWindow", "original", nullptr));
	comboBox_2->setItemText(1, QApplication::translate("MainWindow", "fullscreen", nullptr));

	/* Audio output */
	label_3 = new QLabel("label3");
	comboBox_3 = new QComboBox;
	comboBox_3->addItem(QString());
	comboBox_3->addItem(QString());
	option_layout->addWidget(label_3, 3, 0);
	option_layout->addWidget(comboBox_3, 3, 1);

	label_3->setText(QApplication::translate("MainWindow", "Audio output", nullptr));
	comboBox_3->setItemText(0, QApplication::translate("MainWindow", "hdmi", nullptr));
	comboBox_3->setItemText(1, QApplication::translate("MainWindow", "headphone", nullptr));

	/* Loop count */
	label_4 = new QLabel("label4");
	comboBox_4 = new QComboBox;
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	comboBox_4->addItem(QString());
	option_layout->addWidget(label_4, 4, 0);
	option_layout->addWidget(comboBox_4, 4, 1);

	label_4->setText(QApplication::translate("MainWindow", "Loop count", nullptr));
	comboBox_4->setItemText(0, QApplication::translate("MainWindow", "0 (Infinite)", nullptr));
	comboBox_4->setItemText(1, QApplication::translate("MainWindow", "1", nullptr));
	comboBox_4->setItemText(2, QApplication::translate("MainWindow", "2", nullptr));
	comboBox_4->setItemText(3, QApplication::translate("MainWindow", "3", nullptr));
	comboBox_4->setItemText(4, QApplication::translate("MainWindow", "4", nullptr));
	comboBox_4->setItemText(5, QApplication::translate("MainWindow", "5", nullptr));
	comboBox_4->setItemText(6, QApplication::translate("MainWindow", "6", nullptr));
	comboBox_4->setItemText(7, QApplication::translate("MainWindow", "7", nullptr));
	comboBox_4->setItemText(8, QApplication::translate("MainWindow", "8", nullptr));
	comboBox_4->setItemText(9, QApplication::translate("MainWindow", "9", nullptr));
	comboBox_4->setItemText(10, QApplication::translate("MainWindow", "10", nullptr));
	comboBox_4->setItemText(11, QApplication::translate("MainWindow", "100", nullptr));
	comboBox_4->setItemText(12, QApplication::translate("MainWindow", "1000", nullptr));

	/* button */
	pushButton = new QPushButton(centralwidget);
	button_layout->addWidget(pushButton);
	pushButton->setText(QApplication::translate("MainWindow", "OK", nullptr));
	QMetaObject::connectSlotsByName(this);

	comboBox->setCurrentIndex(g_decode);
	comboBox_2->setCurrentIndex(g_display);
	comboBox_3->setCurrentIndex(g_audio);
	comboBox_4->setCurrentIndex(1);
	connect(pushButton, &QPushButton::clicked, this, &Setting_Dialog::do_update);
}

Setting_Dialog::~Setting_Dialog()
{
	// delete ui;
}

void Setting_Dialog::do_update()
{
	g_decode = comboBox->currentIndex();
	g_display = comboBox_2->currentIndex();
	g_audio = comboBox_3->currentIndex();
	QString	qstr = comboBox_4->currentText();
	g_loop = qstr.toInt();
	QDialog::close();
}