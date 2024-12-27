SUMMARY = "VIA Center"
LICENSE = "CLOSED"

SRC_URI = "file://vcenter.pro \
		file://main.cpp \
		file://mainwindow.cpp \
		file://mainwindow.h \
		file://ui_mainwindow.h \
		file://json.hpp \
		file://default.conf"

DEPENDS += "qtbase"
RDEPENDS_${PN} += "wayland"

S = "${WORKDIR}"

inherit qmake5

do_install() {
	install -d ${D}${bindir}
	install -m 0755 vcenter ${D}${bindir}
	install -d ${D}/etc/vcenter
	install -D -m 0755 ${WORKDIR}/default.conf ${D}/etc/vcenter/default.conf
}