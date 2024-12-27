SUMMARY = "QT Example Recipe"
LICENSE = "CLOSED"

SRC_URI = "file://vtool.pro \
		file://main.cpp \
		file://mainwindow.cpp \
		file://mainwindow.h \
		file://ui_mainwindow.h \
		file://json.hpp \
		file://default.conf \
		file://vab-5000.conf"

DEPENDS += "qtbase"
RDEPENDS_${PN} += "wayland"

S = "${WORKDIR}"

inherit qmake5

do_install() {
	install -d ${D}${bindir}
	install -m 0755 vtool ${D}${bindir}
	install -d ${D}/etc/vtool/conf.d
	install -D -m 0755 ${WORKDIR}/default.conf ${D}/etc/vtool/default.conf
	install -D -m 0755 ${WORKDIR}/vab-5000.conf ${D}/etc/vtool/conf.d/vab-5000.conf
}