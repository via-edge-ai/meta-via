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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <QScreen>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QThread>
#include <QFileDialog>

class VideoThread : public QThread {
	Q_OBJECT
public:
	VideoThread(QObject *parent = nullptr);
	void run();
	void stop();
	bool stopped;

signals:
	void updateRGBImg(QImage img);

private:
	
	
};

class MessageThread : public QThread {
	Q_OBJECT
public:
	MessageThread(QObject *parent = nullptr);
	void run();
	void stop();
	bool stopped;

private:
	
	
};

class PlayerWindow : public QWidget
{
    Q_OBJECT
public:
	PlayerWindow();
	// static gboolean postGstMessage(GstBus * bus, GstMessage * message, gpointer user_data);
	QLabel *myLabel;

	GstState state;
	QString filename;

private slots:
	void onFileClicked() ;
	void onSettingClicked() ;
	void onPlayClicked() ;
	void onStopClicked() ;
	void onAlbumAvaiable(const QString &album);
	void onState(GstState st);
	void refreshSlider();
	void onSeek();
	void onEos();
	// void updateFrame(QImage img);

signals:
	void sigAlbum(const QString &album);
	void sigState(GstState st);
	void sigEos();
  
private:
	QPushButton *fileBt;
	QPushButton *setBt;
	QPushButton *configBt;
	QPushButton *playBt;
	QPushButton *stopBt;
	QWidget *videoWindow;
	QSlider *slider;
	QHBoxLayout *buttonLayout;
	QVBoxLayout *playerLayout;

	gint64 totalDuration;
};
#endif
