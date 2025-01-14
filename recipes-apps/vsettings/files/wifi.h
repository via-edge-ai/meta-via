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
#ifndef WIFI_H
#define WIFI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTextEdit>
#include <QIcon>
#include <QPixmap>
#include <QMessageBox>

#include "switchbutton.h"

namespace Ui {
class MainWindow;
}

class WifiClass : public QMainWindow
{
	Q_OBJECT

public:
	explicit WifiClass(Ui::MainWindow *ui);
	~WifiClass();

	QWidget *page_wifi;
	Switch *switch_wifi;
	QListWidget *wifi_listWidget;
	int wifi_listwidget_item_cnt;
	int skip_event;

	void wifi_show_list();
	void page_enable(int enable);

signals:
	// void sendText(QString str);

public slots:
	void do_wifi_enable(int enable);
	void do_wifi_timer();
	void itemDoubleClicked(QListWidgetItem*);

private:
	Ui::MainWindow *ui;
};

/* Wi-Fi Dialog */
namespace Ui {
class WiFi_Dialog;
}

class WiFi_Dialog : public QDialog
{
	Q_OBJECT

public:
	explicit WiFi_Dialog(QWidget *parent = nullptr);
	~WiFi_Dialog();
	void set_wifi_label(char *str, int level, int in_use);

private:
	Ui::WiFi_Dialog *ui;
	QString device_name;

// 定义信号(信号只需声明无需实现)
signals:
	void sendText(QString str);

private slots:
	void btn1_onBtnClick();
	void btn2_onBtnClick();
	void btn3_onBtnClick();
};

QT_BEGIN_NAMESPACE

class Ui_WiFi_Dialog
{
public:
	QWidget *widget;
	QGridLayout *gridLayout;
	QIcon *icon;
	QLabel *label1;
	QLabel *label;
	QLineEdit *lineEdit;
	QPushButton *pushButton1;
	QPushButton *pushButton2;
	QPushButton *pushButton3;

	void setupUi(QDialog *Dialog)
	{
		if (Dialog->objectName().isEmpty())
			Dialog->setObjectName(QStringLiteral("Dialog"));
		Dialog->resize(250, 120);
		widget = new QWidget(Dialog);
		widget->setObjectName(QStringLiteral("widget"));
		widget->setGeometry(QRect(10, 10, 230, 110));
		gridLayout = new QGridLayout(widget);
		gridLayout->setObjectName(QStringLiteral("gridLayout"));
		gridLayout->setContentsMargins(0, 0, 0, 0);

		label1 = new QLabel("label");
		gridLayout->addWidget(label1, 0, 0, 1, 1);
		gridLayout->setAlignment(label1, Qt::AlignHCenter);

		label = new QLabel("label");
		gridLayout->addWidget(label, 0, 1, 1, 2);

		lineEdit = new QLineEdit(widget);
		lineEdit->setObjectName(QStringLiteral("lineEdit"));
		gridLayout->addWidget(lineEdit, 1, 0, 1, 3);

		pushButton1 = new QPushButton(widget);
		pushButton1->setObjectName(QStringLiteral("pushButton"));
		gridLayout->addWidget(pushButton1, 2, 0, 1, 1);

		pushButton2 = new QPushButton(widget);
		pushButton2->setObjectName(QStringLiteral("pushButton"));
		gridLayout->addWidget(pushButton2, 2, 1, 1, 1);

		pushButton3 = new QPushButton(widget);
		pushButton3->setObjectName(QStringLiteral("pushButton"));
		gridLayout->addWidget(pushButton3, 2, 2, 1, 1);

		retranslateUi(Dialog);

		QMetaObject::connectSlotsByName(Dialog);
	} // setupUi

	void retranslateUi(QDialog *Dialog)
	{
		Dialog->setWindowTitle(QApplication::translate("Dialog", "Password", nullptr));
		pushButton1->setText(QApplication::translate("Dialog", "Delete", nullptr));
		pushButton2->setText(QApplication::translate("Dialog", "Connect", nullptr));
		pushButton3->setText(QApplication::translate("Dialog", "Cancel", nullptr));
	} // retranslateUi
};

namespace Ui {
	class WiFi_Dialog : public Ui_WiFi_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // WIFI_H