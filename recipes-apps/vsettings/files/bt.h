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
#ifndef BT_H
#define BT_H

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>
#include "switchbutton.h"
#include <QThread>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class BT_Scan : public QThread {
	public:
		BT_Scan();
		void run();
		int flag;
};

class BTClass : public QMainWindow
{
	Q_OBJECT

public:
	explicit BTClass(Ui::MainWindow *ui);
	~BTClass();
	QWidget *bt_page;
	Switch *bt_switch;
	QListWidget *bt_listWidget;
	BT_Scan *scan_thread;

	void bt_show_list(void);
	void page_enable(int enable);

public slots:
	void bt_switch_enable(int enable);
	void itemDoubleClicked(QListWidgetItem*);
	void do_list_timer(void);

private:
	Ui::MainWindow *ui;
};

#endif // BT_H