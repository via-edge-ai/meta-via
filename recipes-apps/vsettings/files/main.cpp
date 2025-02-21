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

/*--- History ------------------------------------------------------------------
*
*  1.0.0 (2024-10-01): First release
*
*------------------------------------------------------------------------------*/
#define APP_VERSION	"1.0.0"

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	printf("%s %s\n", argv[0], APP_VERSION);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}