DESCRIPTION = "VIA Key Handler"
LICENSE = "Closed"
#LICENSE = "GPL-2.0-or-later"

inherit systemd

LIC_FILES_CHKSUM = "file://LICENSE;md5=ffa10f40b98be2c2bc9608f56827ed23"

TARGET_CC_ARCH += "${LDFLAGS}"

INSANE_SKIP_${PN} += "ldflags"

SRC_URI = " \
	file://main.cpp \
	file://InputHandler.cpp \
	file://InputHandler.h \
	file://ViaKeyHandler.cpp \
	file://ViaKeyHandler.h \
	file://Makefile \
	file://key_handler.sh \
	file://key_handler.service \
	file://LICENSE \
"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'key_handler.service', '', d)}"

S = "${WORKDIR}"

do_compile() {
    make
}

do_install() {
   install -d ${D}${bindir}
   install -d ${D}${systemd_unitdir}/system

   install -m 0755 ${S}/key_handler ${D}${bindir}
   install -m 0755 ${S}/key_handler.sh ${D}${bindir}
   install -m 0755 ${S}/key_handler.service ${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
	${bindir}/key_handler \
	${bindir}/key_handler.sh \
	${systemd_unitdir}/system/key_handler.service \
"

