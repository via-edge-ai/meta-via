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
#ifndef VCENTER_H
#define VCENTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
	QWidget *centralwidget;
	QWidget *widget;
	QGridLayout *gridLayout;
	QMenuBar *menubar;
	QStatusBar *statusbar;

	void setupUi(QMainWindow *MainWindow)
	{
		if (MainWindow->objectName().isEmpty())
			MainWindow->setObjectName(QStringLiteral("MainWindow"));
		MainWindow->resize(1034, 572);
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
		MainWindow->setSizePolicy(sizePolicy);
		centralwidget = new QWidget(MainWindow);
		centralwidget->setObjectName(QStringLiteral("centralwidget"));
		widget = new QWidget(centralwidget);
		widget->setObjectName(QStringLiteral("widget"));
		widget->setGeometry(QRect(30, 20, 971, 481));
		gridLayout = new QGridLayout(widget);
		gridLayout->setObjectName(QStringLiteral("gridLayout"));
		gridLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
		gridLayout->setContentsMargins(0, 0, 0, 0);
		gridLayout->setRowStretch(0, 1);
		gridLayout->setRowStretch(1, 1);
		gridLayout->setColumnStretch(0, 1);
		gridLayout->setColumnStretch(1, 1);
		gridLayout->setColumnStretch(2, 1);
		gridLayout->setMargin(20);
		gridLayout->setSpacing(20);

		centralwidget->setLayout(gridLayout);
		MainWindow->setCentralWidget(centralwidget);
		menubar = new QMenuBar(MainWindow);
		menubar->setObjectName(QStringLiteral("menubar"));
		menubar->setGeometry(QRect(0, 0, 1034, 21));
		MainWindow->setMenuBar(menubar);
		statusbar = new QStatusBar(MainWindow);
		statusbar->setObjectName(QStringLiteral("statusbar"));
		MainWindow->setStatusBar(statusbar);

		retranslateUi(MainWindow);

		QMetaObject::connectSlotsByName(MainWindow);
	} // setupUi

	void retranslateUi(QMainWindow *MainWindow)
	{
		MainWindow->setWindowTitle(QApplication::translate("MainWindow", "VCenter", nullptr));
	} // retranslateUi
};

namespace Ui {
	class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // VCENTER_H
