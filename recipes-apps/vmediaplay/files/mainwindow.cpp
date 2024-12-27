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
*  1.0.0 (2024-11-21): First release
*
*------------------------------------------------------------------------------*/
#define APP_NAME "VMediaPlayer"
#define APP_VERSION	"1.0.0"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>

#include <gst/video/videooverlay.h>
#include <QApplication>
#include <gst/wayland/wayland.h>
#include "setting_dialog.h"
#include "mainwindow.h"

using namespace std;

string g_platform;
GstElement *pipeline = NULL, *sink = NULL;
GstAppSink *appsink = NULL;

int g_is_audio, g_is_debian;
int g_decode = 0;	// 0:decodebin,1:v4l2h264dec,2:v4l2h265dec,3:v4l2mpeg4dec
int g_display = 0;	// 0:original,1:fullscreen
int g_audio = 0;	// 0:hdmi,1:headphone
int g_loop = 10;		// loop count
int g_media_w, g_media_h;
int g_full_w, g_full_h;
int g_loop_cnt;
int frame_cnt = 0;
PlayerWindow *window;
QTimer *timer;
VideoThread *videoThread;
MessageThread *messageThread;

string exec(string command) {
	char buffer[128];
	string result = "";

	// Open pipe to file
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		return "popen failed!";
	}

	// read till end of process:
	while (!feof(pipe)) {

		// use buffer to read and add to result
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}

	pclose(pipe);
	return result;
}

int get_platform_name(void)
{
	std::size_t pos,rpos;
	string str = exec("fw_printenv boot_conf");

	pos = str.find("#conf-mediatek_");
	if (pos == std::string::npos) {
		return -1;
	}
	rpos = str.find(".dtb#");
	g_platform = str.substr(pos + 15, rpos - pos - 15);

	str = exec("cat /etc/os-release");
	pos = str.find("ID=debian");
	g_is_debian = (pos == std::string::npos) ? 0 : 1;
	return 0;
}

int parse_mdedia_info(QString file_name)
{
	string str, tmp;
	std::size_t pos, rpos;
	
	str = "gst-discoverer-1.0 ";
	str.append(file_name.toStdString());
	str = exec(str);
	pos = str.find("audio");
	g_is_audio = (pos == std::string::npos) ? 0 : 1;
	if ((pos = str.find("Width: ")) != std::string::npos) {
		rpos = str.find("\n");
		tmp = str.substr(pos + 7, rpos - pos - 7);
		g_media_w = stoi(tmp);
	} else {
		printf("%s : %s\n", __func__, str.c_str());
	}

	if ((pos = str.find("Height: ")) != std::string::npos) {
		rpos = str.find("\n");
		tmp = str.substr(pos + 8, rpos - pos - 8);
		g_media_h = stoi(tmp);
	} else {
		printf("%s : %s\n", __func__, str.c_str());
	}
	return 0;
}

void closePipeline()
{
	videoThread->stop();
	messageThread->stop();
	timer->stop();

	if (pipeline) {
    	gst_element_set_state(pipeline, GST_STATE_NULL);
    	
    	GstElement *source;
    	source = gst_bin_get_by_name (GST_BIN (pipeline), "source");
    	gst_element_set_state(source, GST_STATE_NULL);
    	gst_object_unref(source);
    	
		gst_object_unref(pipeline);
		pipeline = NULL;
	}
	
	if (sink) {
		gst_object_unref(sink);
		sink = NULL;
		appsink = NULL;
	}
}

int createPipeline(QString filename, int play)
{
	QString str;

	closePipeline();
	if (!filename.size()) {
		str = "videotestsrc name=source ! appsink name=video-sink";
	} else {
		parse_mdedia_info(filename);

		str = "-v filesrc location=";
		str.append(filename);
		str.append(" name=source");
		str.append(" ! qtdemux name=demux");
		str.append(" demux.video_0 ! queue !");
		switch(g_decode) {
		case 0:
		default:
			str.append(" decodebin !");
			break;
		case 1:
			str.append(" parsebin ! v4l2h264dec !");
			break;
		case 2:
			str.append(" parsebin ! v4l2h265dec !");
			break;
		case 3:
			str.append(" parsebin ! v4l2mpeg4dec !");
			break;
		}
		str.append(" v4l2convert output-io-mode=dmabuf-import capture-io-mode=dmabuf ! video/x-raw,format=(string)BGR ! appsink name=video-sink");
		if (!g_is_audio) // no audio
			goto do_cmd_end;
		str.append(" demux.audio_0 ! queue !");
		str.append(" decodebin !");
		str.append(" audioconvert ! audioresample !");
		if (g_is_debian) { /* debian or ubuntu */
			str.append(" autoaudiosink");
		} else { /* yocto */
			switch(g_audio) {
			case 0:
				str.append(" alsasink device=\"hw:0,5\"");
				break;
			case 1:
				str.append(" alsasink device=\"hw:0,0\"");
				break;
			}
		}
	}
do_cmd_end:
	GError *error = NULL;
	pipeline = gst_parse_launch(str.toStdString().c_str(), &error);
	if (error != NULL) {
		printf("%s fail: %s\n", __func__, error->message);
		g_clear_error(&error);
	}
	sink = gst_bin_get_by_name (GST_BIN (pipeline), "video-sink");
	appsink = GST_APP_SINK(sink);
	if (appsink == NULL) {
		printf("%s sink fail\n");
	}
	
	if (play) {
		GstStateChangeReturn sret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
		if (sret == GST_STATE_CHANGE_FAILURE) {
			printf("%s fail\n", __func__);			
			gst_element_set_state(pipeline, GST_STATE_NULL);
			gst_object_unref(pipeline);
			gst_object_unref(sink);
			// Exit application
			QTimer::singleShot(0, QApplication::activeWindow(), SLOT(quit()));
		}
	}
	videoThread->stopped = false;
	videoThread->start();
	messageThread->stopped = false;
	messageThread->start();
	timer->start(1000);
	return 0;
}

char *gst_state_str[] = {"VOID","NULL","READY","PAUSE","PLAY"};

MessageThread::MessageThread(QObject *parent) : QThread(parent)
{
	stopped = false;
}

void MessageThread::stop()
{
	stopped = true;
}

void MessageThread::run()
{
	GstBus *bus;
	GstMessage *message = NULL;

	while (!stopped) {
		bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
		message = gst_bus_pop(bus);
		if (message != NULL) {
			switch (GST_MESSAGE_TYPE(message)) {
			case GST_MESSAGE_STATE_CHANGED: {
				GstState old_state, new_state, pending_state;
				gst_message_parse_state_changed (message, &old_state, &new_state, &pending_state);
				// printf("[msg] state %s -> %s\n", gst_state_str[old_state], gst_state_str[new_state]);
				window->sigState(new_state);
				break;
			}
			case GST_MESSAGE_TAG: {
				// printf("[msg] TAG\n");
				GstTagList *tags = NULL;
				gst_message_parse_tag(message, &tags);
				gchar *album= NULL;
				if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &album)) {
					window->sigAlbum(album);
					g_free(album);
				}
				gst_tag_list_unref(tags);
				break;
			}
			case GST_MESSAGE_ERROR: {
				// printf("[msg] msg ERROR\n");
				// createPipeline(window->filename, 1);
				gst_element_set_state (pipeline, GST_STATE_NULL);
				break;
			}
			case GST_MESSAGE_EOS: {
				// printf("[msg] msg EOS(%d,%d)\n", g_loop, g_loop_cnt);
				g_loop_cnt++;
				if ((g_loop == 0) || (g_loop_cnt < g_loop)) {
					gst_element_seek_simple (pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH , 1);
					// createPipeline(window->filename, 1);
				} else {
					gst_element_set_state (pipeline, GST_STATE_NULL);
				}
				break;
			}
			default:
				// printf("[msg] msg %d\n", GST_MESSAGE_TYPE(message));
				break;
			}
			gst_message_unref(message);
		} else {
			usleep(15000);
		}
		gst_object_unref(bus);
	}
	stopped = false;
}

VideoThread::VideoThread(QObject *parent) : QThread(parent) 
{
	stopped = false;	
}

void VideoThread::stop()
{
	stopped = true;
}

void VideoThread::run()
{
	GstSample *sample = nullptr;

	while(!stopped) {
		// sample = gst_app_sink_pull_sample(appsink);
		sample = nullptr;
		if (appsink) {
			sample = gst_app_sink_try_pull_sample(appsink, GST_SECOND);
		} else {
			usleep(15000);
			continue;
		}
		
		if (!sample) {
			// printf("sample is NULL\n");
		}else{
			GstMapInfo map;
			// GstCaps *caps = gst_sample_get_caps(sample);
			GError *err = NULL;

			int width = (g_display == 1)? g_full_w : g_media_w;
			width = (width > g_full_w)? g_full_w : width;
			int height = (g_display == 1)? g_full_h : g_media_h;
			height = (height > g_full_h)? g_full_h : height;
			// GstStructure *structure = gst_caps_get_structure(caps, 0);
			GstCaps *capsTo = gst_caps_new_simple("video/x-raw","format", G_TYPE_STRING, "RGB",
				"width", G_TYPE_INT, width, "height", G_TYPE_INT, height, NULL);
			GstSample *convertedSample = gst_video_convert_sample(sample,capsTo,GST_SECOND,&err);
			// gst_caps_unref(caps);
			gst_caps_unref(capsTo);
			gst_sample_unref(sample);

			if (convertedSample == nullptr) {
				//qWarning() << "gst_video_convert_sample Failed:" << err->message;
			}
			else {
				GstBuffer * buffer = gst_sample_get_buffer(convertedSample);
				gst_buffer_map(buffer, &map, GST_MAP_READ);

				QImage img = QImage((const uchar *)map.data, width, height, QImage::Format_RGB888);
				window->myLabel->setPixmap(QPixmap::fromImage(img));
				window->myLabel->resize(img.size());
				window->resize(img.size());

				gst_buffer_unmap (buffer, &map);
				gst_sample_unref(convertedSample); // critical
				gst_buffer_unref(buffer); // warning if no sample_unref
			}
		}
	}
	stopped = false;
}

PlayerWindow::PlayerWindow()
	:state(GST_STATE_NULL)
	,totalDuration(GST_CLOCK_TIME_NONE)
{
	fileBt = new QPushButton("File");
	setBt = new QPushButton("Setting");
	playBt = new QPushButton("Play/Pause");
	stopBt = new QPushButton("Stop");
	videoWindow = new QWidget();
	slider = new QSlider(Qt::Horizontal);
	myLabel = new QLabel(this);

	connect(fileBt, SIGNAL(clicked()), this, SLOT(onFileClicked()));
	connect(setBt, SIGNAL(clicked()), this, SLOT(onSettingClicked()));
	connect(playBt, SIGNAL(clicked()), this, SLOT(onPlayClicked()));
	connect(stopBt, SIGNAL(clicked()), this, SLOT(onStopClicked()));
	connect(slider, SIGNAL(sliderReleased()), this, SLOT(onSeek()));

	buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(fileBt);
	buttonLayout->addWidget(setBt);
	buttonLayout->addWidget(playBt);
	buttonLayout->addWidget(stopBt);
	buttonLayout->addWidget(slider);

	playerLayout = new QVBoxLayout;
	playerLayout->addWidget(myLabel);
	playerLayout->addLayout(buttonLayout);

	this->setLayout(playerLayout);

	connect(timer, SIGNAL(timeout()), this, SLOT(refreshSlider()));
	connect(this, SIGNAL(sigAlbum(QString)), this, SLOT(onAlbumAvaiable(QString)));
	connect(this, SIGNAL(sigState(GstState)), this, SLOT(onState(GstState)));
	connect(this, SIGNAL(sigEos()), this, SLOT(onEos()));
}

void PlayerWindow::onFileClicked() {
	QStringList filenames;
	QFileDialog *dlg=new QFileDialog;

	gst_element_set_state (pipeline, GST_STATE_NULL);

	//QFont font = ui->centralwidget->font();
	//dlg->setFont(font);
	//dlg->setWindowTitle("Open Media");

	QFileInfo fi(filename);
	dlg->setDirectory(fi.absolutePath());

	dlg->setNameFilter("media files (*.mp4 *.avi);;All files (*.*)");
	dlg->setViewMode(QFileDialog::List);
	// dlg->resize(640,400);
	if(dlg->exec()==QDialog::Accepted) {
		filenames = dlg->selectedFiles();
		filename = filenames[0];
	}
	createPipeline(filename, 1);
	g_loop_cnt = 0;
}

void PlayerWindow::onSettingClicked() {
	gst_element_set_state (pipeline, GST_STATE_NULL);
	Setting_Dialog *subwindow = new Setting_Dialog(this);
	int ref = subwindow->exec();
}

void PlayerWindow::onPlayClicked() {
	GstState state, pending;

	gst_element_get_state(pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
	switch (state) {
	case GST_STATE_PLAYING:
		gst_element_set_state (pipeline, GST_STATE_PAUSED);
		break;
	case GST_STATE_PAUSED:
		gst_element_set_state (pipeline, GST_STATE_PLAYING);
		break;
	default:
		g_loop_cnt = 0;
		createPipeline(filename, 1);
		break;
	}
}

void PlayerWindow::onStopClicked() {
	closePipeline();
	//gst_element_set_state (pipeline, GST_STATE_NULL);
	//gst_object_unref(pipeline);
	//gst_object_unref(sink);
	g_loop_cnt = 0;
}

void PlayerWindow::onAlbumAvaiable(const QString &album) {
	setWindowTitle(album);
}

void PlayerWindow::onState(GstState st) {
	if (state != st) {
		state = st;
		if (state == GST_STATE_PLAYING){
			timer->start(1000);
		}
		if (state < GST_STATE_PAUSED){
			timer->stop();
		}
	}
}

void PlayerWindow::refreshSlider() {
	GstState state, pending;
	gint64 current = GST_CLOCK_TIME_NONE;

	gst_element_get_state(pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
	switch (state) {
	case GST_STATE_PLAYING:
		if (!GST_CLOCK_TIME_IS_VALID(totalDuration)) {
			if (gst_element_query_duration (pipeline, GST_FORMAT_TIME, &totalDuration)) {
				slider->setRange(0, totalDuration/GST_SECOND);
			}
		}
		if (gst_element_query_position (pipeline, GST_FORMAT_TIME, &current)) {
			// g_print("%ld / %ld\n", current/GST_SECOND, totalDuration/GST_SECOND);
			slider->setValue(current/GST_SECOND);
		}
		break;
	case GST_STATE_NULL:
		totalDuration = GST_CLOCK_TIME_NONE;
		slider->setRange(0, 0);
		slider->setValue(0);
		break;
	}
}

void PlayerWindow::onSeek() {
	gint64 pos = slider->sliderPosition();
	// g_print("seek: %ld\n", pos);
	gst_element_seek_simple (pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH ,
			pos * GST_SECOND);
}

void PlayerWindow::onEos() {
	gst_element_set_state (pipeline, GST_STATE_NULL);
}

int main(int argc, char *argv[])
{
	printf("%s %s\n", argv[0], APP_VERSION);
	
	if (get_platform_name()) {
		qDebug("*E* could not find platform\n");
		return -1;
	}

	gst_init (&argc, &argv);
	QApplication app(argc, argv);
	app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit ()));

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	g_full_w = screenGeometry.width() - 32;
	g_full_h = screenGeometry.height() - 80;

	timer = new QTimer();

	window = new PlayerWindow();
	window->setGeometry(0, 0, 720, 480);
	window->show();

	videoThread = new VideoThread(window);
	messageThread = new MessageThread(window);

	window->filename = "";
	// window->filename = "/mnt/sda1/test1_720.mp4";	
	// createPipeline(window->filename, 1);

	int ret = app.exec();

	window->hide();
	closePipeline();
	return ret;
}