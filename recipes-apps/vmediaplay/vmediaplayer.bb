DESCRIPTION = "VIA Media Player"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://vmediaplayer.pro \
		file://mainwindow.cpp \
		file://mainwindow.h \
		file://setting_dialog.cpp \
		file://setting_dialog.h"

DEPENDS += "qtbase gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad glib-2.0"
RDEPENDS_${PN} += "wayland"

S = "${WORKDIR}"

inherit qmake5

do_install() {
    install -d ${D}${bindir}
	install -m 0755 vmediaplayer ${D}${bindir}
}

FILES:${PN} += " \
	${bindir}/vmediaplayer \
"
