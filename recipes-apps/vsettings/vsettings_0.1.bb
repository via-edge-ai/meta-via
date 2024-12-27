SUMMARY = "VIA Settings"
LICENSE = "CLOSED"

SRC_URI = "file://vsettings.pro \
		file://main.cpp \
		file://mainwindow.cpp \
		file://mainwindow.h \
		file://ui_mainwindow.h \
		file://wifi.cpp \
		file://wifi.h \
		file://bt.cpp \
		file://bt.h \
		file://display.cpp \
		file://display.h \
		file://about.cpp \
		file://about.h \
		file://switchbutton.h \
		file://switchbutton.cpp \
		file://wifi0.png \
		file://wifi1.png \
		file://wifi2.png \
		file://wifi3.png \
		file://wifi4.png"

DEPENDS += "qtbase"
RDEPENDS_${PN} += "wayland"

S = "${WORKDIR}"

inherit qmake5

do_install() {
	install -d ${D}${bindir}
	install -m 0755 vsettings ${D}${bindir}
	install -d ${D}/etc/vsettings
	install -D -m 0755 ${WORKDIR}/wifi0.png ${D}/etc/vsettings/wifi0.png
	install -D -m 0755 ${WORKDIR}/wifi1.png ${D}/etc/vsettings/wifi1.png
	install -D -m 0755 ${WORKDIR}/wifi2.png ${D}/etc/vsettings/wifi2.png
	install -D -m 0755 ${WORKDIR}/wifi3.png ${D}/etc/vsettings/wifi3.png
	install -D -m 0755 ${WORKDIR}/wifi4.png ${D}/etc/vsettings/wifi4.png
}