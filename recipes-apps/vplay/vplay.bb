DESCRIPTION = "VIA Player"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://vplay.sh \
		file://vplay.pro \
		file://main.cpp \
		file://mainwindow.cpp \
		file://mainwindow.h \
		file://ui_mainwindow.h"

DEPENDS += "qtbase"
RDEPENDS_${PN} += "wayland"

S = "${WORKDIR}"

inherit qmake5

do_install() {
    install -d ${D}${bindir}
    install -D -m 0755 ${WORKDIR}/vplay.sh ${D}${bindir}/vplay.sh
	install -m 0755 vplay ${D}${bindir}
}

FILES:${PN} += " \
	${bindir}/vplay.sh \
	${bindir}/vplay \
"
