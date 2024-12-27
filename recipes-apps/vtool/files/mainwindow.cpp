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
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string.h>
#include <list>

#include <QLabel>
#include <QVBoxLayout>
#include <QDialog>
#include <QScreen>
#include <QFont>
#include <QMessageBox>
#include <QPushButton>
#include <QString>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

#define CONFIG_TRANSFORMA
#define APP_NAME "VTool"

enum btn_type_t {
	BTN_CHKBOX,
	BTN_RADIO,
	BTN_MAX
};

#define BIT(n) (0x01 << n)
#define DTBO_FLAGS_CHECKED  BIT(0)
#define DTBO_FLAGS_ENABLED	BIT(1)
#define DTBO_FLAGS_HIDED	BIT(2)

struct dtbo_s {
	string name;
	string alias;
	enum btn_type_t btn_type;
	void *btn;
	int mask;
	int flag;
};

struct dtb_group_s {
	string label;
	string prefix;
	string default_id;
	string default_alias;
	enum btn_type_t btn_type;

	std::vector<dtbo_s> whitelist;
	std::vector<std::string> blacklist;
};

struct global_var_s {
	string platform;
	string config_file;
	std::vector<std::string> img_dtbo_list;
	std::list<dtbo_s> dtbo_list;
	int title_platform;
	float scale;
	int debug;
	int dtbo_alias;
} g;

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

enum btn_type_t get_btn_type(string str)
{
	if (str.size() == 0)
		return BTN_MAX;
	if (str.compare("chkbox") == 0)
		return BTN_CHKBOX;
	if (str.compare("radio") == 0)
		return BTN_RADIO;
	return BTN_MAX;
}

int get_platform(void)
{
	std::size_t pos,rpos;
	string str = exec("fw_printenv boot_conf");

	pos = str.find("#conf-mediatek_");
	if (pos == std::string::npos) {
		return -1;
	}
	rpos = str.find(".dtb#");
	g.platform = str.substr(pos + 15, rpos - pos - 15);
	return 0;
}

int get_config(void)
{
	g.config_file = "/etc/vtool/conf.d/" + g.platform + ".conf";
	if( std::filesystem::exists(g.config_file.c_str()) != true ) {
		g.config_file = "/etc/vtool/default.conf";
		if( std::filesystem::exists(g.config_file.c_str()) != true ) {
			return -1;
		}
	}

	ifstream fjson(g.config_file.c_str());
	stringstream buffer;
	buffer << fjson.rdbuf();
	auto json = nlohmann::json::parse(buffer.str());
	int val;

	if (!json["debug"].is_null()) {
		json["debug"].get_to(val);
		g.debug = val;
	}

	if (!json["title_platform"].is_null()) {
		json["title_platform"].get_to(val);
		g.title_platform = val;
	}

	g.dtbo_alias = 1;
	if (!json["dtbo_alias"].is_null()) {
		json["dtbo_alias"].get_to(val);
		g.dtbo_alias = val;
	}

	return 0;
}

int get_group_config(int idx, struct dtb_group_s *grp)
{
	ifstream fjson(g.config_file.c_str());
	stringstream buffer;
	buffer << fjson.rdbuf();
	auto json = nlohmann::json::parse(buffer.str());
	int cnt = 0;

	grp->whitelist.clear();
	grp->blacklist.clear();
	memset(grp, 0, sizeof(struct dtb_group_s));

	for (auto group : json["Groups"]) {
		string str;

		if (idx != cnt) {
			cnt++;
			continue;
		}

		if (!group["label"].is_null())
			group["label"].get_to(grp->label);

		if (!group["prefix"].is_null()) {
			group["prefix"].get_to(str);
			if (str.size())
				grp->prefix = str;
		}

		if (!group["btn_type"].is_null())
			group["btn_type"].get_to(str);
		grp->btn_type = get_btn_type(str);

		if (!group["default"].is_null()) {
			auto p = group["default"];
			if (!p["id"].is_null()) {
				p["id"].get_to(str);
				if (str.size())
					grp->default_id = str;
			}
			if (!p["alias"].is_null()) {
				p["alias"].get_to(str);
				if (str.size())
					grp->default_alias = str;
			}
		}

		if (!group["whitelist"].is_null()) {
			for (auto l : group["whitelist"]) {
				struct dtbo_s p;
				int val;

				memset((void *)&p, 0, sizeof(struct dtbo_s));

				if (!l["id"].is_null()) {
					l["id"].get_to(str);
					if (str.size())
						p.name = str;
				}

				if (!l["alias"].is_null()) {
					l["alias"].get_to(str);
					if (str.size())
						p.alias = str;
				}

				if (!l["checked"].is_null()) {
					p.mask |= DTBO_FLAGS_CHECKED;
					l["checked"].get_to(val);
					p.flag |= (val) ? DTBO_FLAGS_CHECKED : 0;
				}

				if (!l["enabled"].is_null()) {
					p.mask |= DTBO_FLAGS_ENABLED;
					l["enabled"].get_to(val);
					p.flag |= (val) ? DTBO_FLAGS_ENABLED : 0;
				}

				if (!l["hided"].is_null()) {
					p.mask |= DTBO_FLAGS_HIDED;
					l["hided"].get_to(val);
					p.flag |= (val) ? DTBO_FLAGS_HIDED : 0;
				}

				grp->whitelist.push_back(p);
			}
		}

		if (!group["blacklist"].is_null()) {
			for (auto l : group["blacklist"]) {
				l.get_to(str);
				if (str.length())
					grp->blacklist.push_back(str);
			}
		}

		if (g.debug) {
			cout << "===== " << cnt << " =====\n";
			cout << " label:"	<< grp->label << "\n";
			cout << " prefix:"	<< grp->prefix << "\n";
			cout << " default id:"	<< grp->default_id << "\n";
			cout << " default alias:"	<< grp->default_alias << "\n";
			cout << " btn type:"	<< grp->btn_type << "\n";

			cout << " --- whitelist ---"	<< "\n";
			for (std::vector<struct dtbo_s>::iterator it = grp->whitelist.begin(); it != grp->whitelist.end(); it++) {
				qDebug("%s, %s, 0x%x, 0x%x\n", (char *) it->name.c_str(), it->alias.c_str(), it->mask, it->flag);
			}
			cout << " --- blacklist ---"	<< "\n";
			for (std::vector<std::string>::iterator it = grp->blacklist.begin(); it != grp->blacklist.end(); it++) {
				qDebug("%s\n", (char *) it[0].c_str());
			}
			cout << "========== \n";
		}
		return 0;
	}
	return -1;
}

void get_img_dtb(void)
{
	// const char* filename = "/boot/fitImage";
	const char* filename = "/dev/mmcblk0p9";
	const char* search_prefix = "Xfdt-";
	size_t search_prefix_size = strlen(search_prefix);
	const char* search_postfix = ".dtb";
	size_t search_postfix_size = strlen(search_postfix);

	std::ifstream file(filename, std::ios::binary);
	if (file) {
		file.seekg(0, std::ios::end);
		size_t file_size = file.tellg();
		file.seekg(0, std::ios::beg);
		std::string file_content;
		file_content.reserve(file_size);
		char buffer[16384];
		std::streamsize chars_read;

		while (file.read(buffer, sizeof buffer), chars_read = file.gcount())
			file_content.append(buffer, chars_read);

		if (file.eof()) {
			for (std::string::size_type offset = 0, found_at, found_end;
				file_size > offset && (found_at = file_content.find(search_prefix, offset)) != std::string::npos;
				offset = found_at + search_prefix_size) {
				if (found_at && (found_end = file_content.find(search_postfix, found_at)) != std::string::npos) {
					if ((found_end - found_at) < 50) {
						char buf[50];

						file_content.copy(buf, found_end - found_at - 5, found_at + 5);
						buf[found_end - found_at - 5] = 0;
						string str = buf;
						g.img_dtbo_list.push_back(str);
					}
				}
			}
		}
	}

	if (g.debug) {
		cout << "===== img dtbo list ===== \n";
		for (std::vector<std::string>::iterator it = g.img_dtbo_list.begin(); it != g.img_dtbo_list.end(); it++) {
			qDebug("%s ", (char *) it[0].c_str());
		}
		cout << "\n ========== \n";
	}
}

void MainWindow::dtbo_save()
{
	string list_dtbo_str = "fw_setenv list_dtbo \"";
	string boot_conf_str = "fw_setenv boot_conf \"";
	int isChecked;

	boot_conf_str.append("#conf-mediatek_");
	boot_conf_str.append(g.platform.c_str());
	boot_conf_str.append(".dtb");

	for (std::list<struct dtbo_s>::iterator it = g.dtbo_list.begin(); it != g.dtbo_list.end(); it++) {
		isChecked = 0;

		if (it->name.size() == 0)
			continue;

		if (it->btn == 0) {
			isChecked = (it->flag & DTBO_FLAGS_CHECKED) ? 1 : 0;
		} else {
			if (it->btn_type == BTN_CHKBOX) {
				QCheckBox *checkBox = (QCheckBox *) it->btn;
				isChecked = checkBox->isChecked();
			}

			if (it->btn_type == BTN_RADIO) {
				QRadioButton *radioButton = (QRadioButton *) it->btn;
				isChecked = radioButton->isChecked();
			}
		}

		// qDebug("%s : %d\n", it->name.c_str(), isChecked);
		if (isChecked) {
			list_dtbo_str.append(it->name.c_str());
			list_dtbo_str.append(".dtbo ");

			boot_conf_str.append("#conf-");
			boot_conf_str.append(it->name.c_str());
			boot_conf_str.append(".dtbo");
		}
	}

	list_dtbo_str.pop_back();
	list_dtbo_str.append("\"");
	boot_conf_str.append("\"");

	if (g.debug) {
		qDebug("%s\n", list_dtbo_str.c_str());
		qDebug("%s\n", boot_conf_str.c_str());
	}
	exec(list_dtbo_str);
	exec(boot_conf_str);

	QMessageBox msgbox;
	msgbox.setWindowTitle(APP_NAME);
	QPushButton *rebootButton = msgbox.addButton(tr("Reboot"), QMessageBox::ActionRole);
	QPushButton *ignoreButton = msgbox.addButton(QMessageBox::Ignore);

	msgbox.setText("<p style='text-align: left;'>In order for the changes to take effect,</p>"
					"<p style='text-align: left;'>you must reboot the system.</p>");
	msgbox.setInformativeText("<p style='text-align: left;'>Do you want to reboot now?</p>");

	string str = "QLabel{min-width:";
	str += to_string((int)(20 * 12 * g.scale));
	str += "px; font-size: ";
	str += to_string((int)(12 * g.scale));
	str += "px;} QPushButton{ width:";
	str += to_string((int)(100 * g.scale));
	str += "px; font-size: ";
	str += to_string((int)(12 * g.scale));
	str += "px; }";
	// qDebug("scale %f, %s\n", scale, str.c_str());
	msgbox.setStyleSheet(QString::fromStdString(str));

	msgbox.setDefaultButton(QMessageBox::Ignore);
	msgbox.exec();
	if (msgbox.clickedButton() == rebootButton) {
		exec("sudo reboot");
	}
}

void *add_btn(enum btn_type_t type, QFrame *frame, int idx, struct dtbo_s *it)
{
	void *btn;
	char *str = it->name.data();
	bool flag = (it->mask & DTBO_FLAGS_CHECKED) ? ((it->flag & DTBO_FLAGS_CHECKED) != 0) : 0;
	bool enable = (it->mask & DTBO_FLAGS_ENABLED) ? ((it->flag & DTBO_FLAGS_ENABLED) != 0) : 1;
	bool hide = (it->mask & DTBO_FLAGS_HIDED) ? ((it->flag & DTBO_FLAGS_HIDED) != 0) : 0;

	if (g.dtbo_alias || !it->name.size()) {
		if (it->alias.size())
			str = it->alias.data();
	}

	switch(type) {
	case BTN_CHKBOX:
		QCheckBox *checkBox;

		checkBox = new QCheckBox(frame);
		checkBox->setObjectName(str);
		checkBox->setGeometry(QRect(10 * g.scale, (5 + 20 * idx) * g.scale, 300 * g.scale, 20 * g.scale));
		checkBox->setText(QApplication::translate("MainWindow", str, nullptr));
		checkBox->setChecked(flag);
		checkBox->setEnabled(enable);

		//checkBox->setStyleSheet("QCheckBox::indicator { width: 20px; height: 20px; }");
		btn = (void *) checkBox;
		break;
	case BTN_RADIO:
		QRadioButton *radioButton;

		radioButton = new QRadioButton(frame);
		radioButton->setObjectName(str);
		radioButton->setGeometry(QRect(10 * g.scale, (5 + 20 * idx) * g.scale, 300 * g.scale, 20 * g.scale));
		radioButton->setText(QApplication::translate("MainWindow", str, nullptr));
		radioButton->setChecked(flag);
		radioButton->setEnabled(enable);

		//radioButton->setStyleSheet("QRadioButton::indicator { width: 10px; height: 10px; }");
		btn = (void *) radioButton;
		break;
	default:
		break;
	}
	return btn;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	struct dtb_group_s grp;
	int frame_idx = 0, btn_idx = 0;
	int offset_l = 10, offset_r = 10;
	int i;
	string tmp_string;

	string list_dtbo_str = exec("fw_printenv list_dtbo");
	string boot_conf_str = exec("fw_printenv boot_conf");

	if (get_platform()) {
		qDebug("*E* could not find platform\n");
		QMainWindow::close();
		return;
	}

	if (get_config()) {
		qDebug("*E* no config file\n");
		QMainWindow::close();
		return;
	}

	get_img_dtb();

	/* windows ui */
	ui->setupUi(this);

	tmp_string = APP_NAME;
	if (g.title_platform) {
		tmp_string.append(" (");
		tmp_string.append(g.platform);
		tmp_string.append(")");
	}
	MainWindow::setWindowTitle(QApplication::translate("MainWindow", tmp_string.c_str(), nullptr));

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	int height = screenGeometry.height();
	int width = screenGeometry.width();

	g.scale = width / 32;
	g.scale = (g.scale < 25) ? 0.5 : g.scale * 0.02;
	// qDebug("screen %dx%d, scale %f\n", width, height, scale);

	QFont font = ui->centralwidget->font();
	font.setPointSize(10 * g.scale);
	ui->centralwidget->setFont(font);

	/* skip platform dtb */
	for (std::vector<std::string>::iterator it = g.img_dtbo_list.begin(); it != g.img_dtbo_list.end(); it++) {
		if (strncmp("mediatek_", (char *) it[0].c_str(), 9) == 0) {
			g.img_dtbo_list.erase(it);
			break;
		}
	}

	/* parse group dtbo */
	for( i = 0; ; i++) {
		struct dtb_group_s *p = &grp;
		QLabel *label;
		QFrame *frame;
		QScrollArea *area;
		int list_cnt = g.dtbo_list.size();

		if (get_group_config(i, &grp) != 0)
			break;

		/* blacklist */
		if (!p->blacklist.empty()) {
			for (std::vector<std::string>::iterator it1 = p->blacklist.begin(); it1 != p->blacklist.end(); it1++) {
				for (std::vector<std::string>::iterator it2 = g.img_dtbo_list.begin(); it2 != g.img_dtbo_list.end(); it2++) {
					if (strcmp((char *) it1[0].c_str(), (char *) it2[0].c_str()) == 0) {
						g.img_dtbo_list.erase(it2);
						break;
					}
				}
			}
		}

		/* whitelist */
		if (!p->whitelist.empty()) {
			for (std::vector<struct dtbo_s>::iterator it1 = p->whitelist.begin(); it1 != p->whitelist.end(); it1++) {
				for (std::vector<std::string>::iterator it2 = g.img_dtbo_list.begin(); it2 != g.img_dtbo_list.end(); it2++) {
					if (strcmp((char *) it1->name.c_str(), (char *) it2[0].c_str()) == 0) {
						struct dtbo_s dtbo;

						dtbo.name = it1->name;
						dtbo.alias = it1->alias;
						dtbo.btn_type = p->btn_type;
						dtbo.mask = it1->mask;
						dtbo.flag = it1->flag;
						dtbo.btn = 0;

						g.dtbo_list.push_back(dtbo);
						g.img_dtbo_list.erase(it2);
						break;
					}
				}
			}
		}

		/* prefix */
		if (p->prefix.size()) {
			int match;
			do {
				match = 0;
				for (std::vector<std::string>::iterator it = g.img_dtbo_list.begin(); it != g.img_dtbo_list.end(); it++) {
					if (strncmp((char *) p->prefix.c_str(), (char *) it[0].c_str(), p->prefix.size()) == 0) {
						struct dtbo_s dtbo;

						dtbo.name = it[0];
						dtbo.btn = 0;
						/* has whitelist: clean prefix, no whitelist: add prefix */
						if (p->whitelist.empty()) {
							g.dtbo_list.push_back(dtbo);
						}
						g.img_dtbo_list.erase(it);
						match = 1;
						break;
					}
				}
			} while(match);
		} else {
			/* push all remain dtbo to group */
			if (p->whitelist.empty()) {
				while(g.img_dtbo_list.size()) {
					for (std::vector<std::string>::iterator it = g.img_dtbo_list.begin(); it != g.img_dtbo_list.end(); it++) {
						struct dtbo_s dtbo;

						dtbo.name = it[0];
						dtbo.btn = 0;
						g.dtbo_list.push_back(dtbo);
						g.img_dtbo_list.erase(it);
						break;
					}
				}
			}
		}

		if (g.dtbo_list.size() != list_cnt) {
			int default_match = 0;

			/* default */
			if (p->default_id.size()) {
				int cnt = 0;

				for (std::list<struct dtbo_s>::iterator it = g.dtbo_list.begin(); it != g.dtbo_list.end(); it++) {
					if (cnt < list_cnt) {
						cnt++;
						continue;
					}
					if (strcmp((char *) it->name.c_str(), (char *) p->default_id.c_str()) == 0) {
						// qDebug("default exist %s\n", it->name.c_str());
						it->mask |= DTBO_FLAGS_CHECKED;
						it->flag |= DTBO_FLAGS_CHECKED;
						default_match = 1;
						break;
					}
				}
			}

			if (default_match == 0) {
				if (p->default_id.size() || p->default_alias.size()) {
					struct dtbo_s dtbo;

					dtbo.name = p->default_id;
					dtbo.alias = p->default_alias;
					dtbo.btn_type = p->btn_type;
					dtbo.mask |= DTBO_FLAGS_CHECKED;
					dtbo.flag |= DTBO_FLAGS_CHECKED;

					// qDebug("default insert %s, btn %d\n", dtbo.name.c_str(), dtbo.btn_type);
					list<struct dtbo_s>::iterator it = g.dtbo_list.begin();

				    advance(it, list_cnt);
					g.dtbo_list.insert(it, dtbo);
				}
			}

			/* ui */
			if (p->btn_type == BTN_MAX)
				continue;

			bool enable = true;
			int cnt = 0;
			btn_idx = 0;
			for (std::list<struct dtbo_s>::iterator it = g.dtbo_list.begin(); it != g.dtbo_list.end(); it++) {
				if (cnt < list_cnt) {
					cnt++;
					continue;
				}

				if (strstr(list_dtbo_str.c_str(), it->name.c_str()) != 0) {
					it->mask |= DTBO_FLAGS_CHECKED;
					it->flag |= DTBO_FLAGS_CHECKED;
				}

				bool hide = (it->mask & DTBO_FLAGS_HIDED) ? ((it->flag & DTBO_FLAGS_HIDED) != 0) : 0;
				if (hide)
					continue;

				if (btn_idx == 0) {
					label = new QLabel( p->label.c_str(), ui->centralwidget);
					label->setGeometry(20 * g.scale, 20 * g.scale, 100 * g.scale, 20 * g.scale);

					frame = new QFrame(ui->centralwidget);
					frame->setObjectName(QStringLiteral("frame"));
					frame->setGeometry(QRect(20 * g.scale, 20 * g.scale, 300 * g.scale, 120 * g.scale));
					frame->setFrameShape(QFrame::StyledPanel);
					frame->setFrameShadow(QFrame::Raised);

					area = new QScrollArea(ui->centralwidget);
					area->setGeometry(20 * g.scale,20 * g.scale,300 * g.scale,120 * g.scale);
					area->setWidget(frame);
				}

				it->btn = add_btn(p->btn_type, frame, btn_idx, (struct dtbo_s *) &*it);
				btn_idx++;
			}

			if (btn_idx == 0)
				continue;

			int x, y, h;

			h = 10 * g.scale + 20 * btn_idx * g.scale;
			if (frame_idx % 2) {
				x = 350 * g.scale;
				y = offset_r;
				offset_r += (h + 30 * g.scale);
			} else {
				x = 20 * g.scale;
				y = offset_l;
				offset_l += (h + 30 * g.scale);
			}

			label->setGeometry(QRect( x, y, 200 * g.scale, 20 * g.scale));
			frame->setGeometry(QRect( x, y + 10 * g.scale, 300 * g.scale, h));
			area->setGeometry(QRect( x, y + 20 * g.scale, 300 * g.scale + 2, h + 2));
			frame_idx++;
		}
	}

	if (g.debug) {
		qDebug("final: img dtbo %d, dtbo %d\n", g.img_dtbo_list.size(), g.dtbo_list.size());
		for (std::vector<std::string>::iterator it = g.img_dtbo_list.begin(); it != g.img_dtbo_list.end(); it++) {
			qDebug("%s\n", it[0].c_str());
		}
	}

	offset_l += 50 * g.scale;
	offset_r += 50 * g.scale;
	i = ((offset_l > offset_r) ? offset_l : offset_r);
	if (frame_idx == 1) {
		ui->QuitButton->setGeometry(QRect(100 * g.scale, i - 50 * g.scale, 72 * g.scale, 25 * g.scale));
		ui->SaveButton->setGeometry(QRect(200 * g.scale, i - 50 * g.scale, 72 * g.scale, 25 * g.scale));
		MainWindow::resize(335 * g.scale, i + 10);
		MainWindow::setFixedSize(335 * g.scale, i + 10);
	} else {
		ui->QuitButton->setGeometry(QRect(400 * g.scale, i - 50 * g.scale, 72 * g.scale, 25 * g.scale));
		ui->SaveButton->setGeometry(QRect(500 * g.scale, i - 50 * g.scale, 72 * g.scale, 25 * g.scale));
		MainWindow::resize(670 * g.scale, i + 10);
		MainWindow::setFixedSize(670 * g.scale, i + 10);
	}

	connect(ui->QuitButton, &QPushButton::clicked, this, &QMainWindow::close);
	connect(ui->SaveButton, &QPushButton::clicked, this, &MainWindow::dtbo_save);
}

MainWindow::~MainWindow()
{
	delete ui;
}