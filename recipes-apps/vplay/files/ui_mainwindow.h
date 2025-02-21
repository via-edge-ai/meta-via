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
#ifndef VPLAY_H
#define VPLAY_H

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

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
	QWidget *centralwidget;
	QComboBox *comboBox;
	QLabel *label;
	QLabel *label_2;
	QComboBox *comboBox_2;
	QLabel *label_3;
	QComboBox *comboBox_3;
	QLabel *label_4;
	QComboBox *comboBox_4;
	// QLineEdit *lineEdit;
	QLabel *label_5;
	QPushButton *pushButton;
	QPushButton *pushButton_2;
	QPushButton *pushButton_3;
	QLabel *label_7;
	QLineEdit *lineEdit_2;
	QTextBrowser *textBrowser;
	QToolButton *toolButton;
	QMenuBar *menubar;
	QStatusBar *statusbar;
	QVBoxLayout *main_layout;
	QGridLayout *option_layout;
	QHBoxLayout *button_layout;

	void setupUi(QMainWindow *MainWindow)
	{
		if (MainWindow->objectName().isEmpty())
			MainWindow->setObjectName(QStringLiteral("MainWindow"));
		//MainWindow->resize(500, 550);
		//MainWindow->setFixedSize(500, 550);
		centralwidget = new QWidget(MainWindow);
		centralwidget->setObjectName(QStringLiteral("centralwidget"));
        main_layout = new QVBoxLayout();
		main_layout->setMargin(20);
		main_layout->setSpacing(20);
		centralwidget->setLayout(main_layout);

		option_layout = new QGridLayout();
		button_layout = new QHBoxLayout;
		main_layout->addLayout(option_layout);
		main_layout->addLayout(button_layout);

		/* File */
		label_7 = new QLabel("file");
		lineEdit_2 = new QLineEdit();
		toolButton = new QToolButton;
		option_layout->addWidget(label_7, 0, 0);
		option_layout->addWidget(lineEdit_2, 0, 1);
		option_layout->addWidget(toolButton, 0, 2);

		/* Codec */
		label = new QLabel("label");
		comboBox = new QComboBox;
		comboBox->addItem(QString());
		comboBox->addItem(QString());
		comboBox->addItem(QString());
		comboBox->addItem(QString());
		option_layout->addWidget(label, 1, 0);
		option_layout->addWidget(comboBox, 1, 1);

		/* Display */
		label_2 = new QLabel("label2");
		comboBox_2 = new QComboBox;
		comboBox_2->addItem(QString());
		comboBox_2->addItem(QString());
		option_layout->addWidget(label_2, 2, 0);
		option_layout->addWidget(comboBox_2, 2, 1);

		/* Audio output */
		label_3 = new QLabel("label3");
		comboBox_3 = new QComboBox;
		comboBox_3->addItem(QString());
		comboBox_3->addItem(QString());
		option_layout->addWidget(label_3, 3, 0);
		option_layout->addWidget(comboBox_3, 3, 1);

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

		/* Command */
		label_5 = new QLabel("label5");
		textBrowser = new QTextBrowser;
		option_layout->addWidget(label_5, 5, 0, Qt::AlignTop);
		option_layout->addWidget(textBrowser, 5, 1);

		/* button */
		pushButton = new QPushButton(centralwidget);
		pushButton_2 = new QPushButton(centralwidget);
		pushButton_3 = new QPushButton(centralwidget);
		button_layout->addWidget(pushButton_3);
		button_layout->addWidget(pushButton_2);
		button_layout->addWidget(pushButton);

		MainWindow->setCentralWidget(centralwidget);
		menubar = new QMenuBar(MainWindow);
		menubar->setObjectName(QStringLiteral("menubar"));
		menubar->setGeometry(QRect(0, 0, 800, 21));
		MainWindow->setMenuBar(menubar);
		statusbar = new QStatusBar(MainWindow);
		statusbar->setObjectName(QStringLiteral("statusbar"));
		MainWindow->setStatusBar(statusbar);

		retranslateUi(MainWindow);

		QMetaObject::connectSlotsByName(MainWindow);
	} // setupUi

	void retranslateUi(QMainWindow *MainWindow)
	{
		MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
		comboBox->setItemText(0, QApplication::translate("MainWindow", "decodebin", nullptr));
		comboBox->setItemText(1, QApplication::translate("MainWindow", "v4l2h264dec", nullptr));
		comboBox->setItemText(2, QApplication::translate("MainWindow", "v4l2h265dec", nullptr));
		comboBox->setItemText(3, QApplication::translate("MainWindow", "v4l2mpeg4dec", nullptr));

		label->setText(QApplication::translate("MainWindow", "Codec", nullptr));
		label_2->setText(QApplication::translate("MainWindow", "Display", nullptr));
		comboBox_2->setItemText(0, QApplication::translate("MainWindow", "original", nullptr));
		comboBox_2->setItemText(1, QApplication::translate("MainWindow", "fullscreen", nullptr));

		label_3->setText(QApplication::translate("MainWindow", "Audio output", nullptr));
		comboBox_3->setItemText(0, QApplication::translate("MainWindow", "hdmi", nullptr));
		comboBox_3->setItemText(1, QApplication::translate("MainWindow", "headphone", nullptr));

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

		label_4->setText(QApplication::translate("MainWindow", "Loop count", nullptr));
		label_5->setText(QApplication::translate("MainWindow", "Command", nullptr));
		pushButton->setText(QApplication::translate("MainWindow", "Play", nullptr));
		pushButton_2->setText(QApplication::translate("MainWindow", "Quit", nullptr));
		pushButton_3->setText(QApplication::translate("MainWindow", "Copy", nullptr));
		label_7->setText(QApplication::translate("MainWindow", "File", nullptr));
		toolButton->setText(QApplication::translate("MainWindow", "...", nullptr));
	} // retranslateUi
};

namespace Ui {
	class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // VPLAY_H
