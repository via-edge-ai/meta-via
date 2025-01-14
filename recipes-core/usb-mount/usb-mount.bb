DESCRIPTION = "USB auto-mount service"
LICENSE = "CLOSED"

inherit systemd

SRC_URI = " \
	file://usb-mount@.service \
	file://usb-mount.sh \
	file://88-usb-mount.rules \
"

#RDEPENDS:${PN} += "bash"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'usb-mount@.service', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "disable"

do_install() {
    install -D -m 0755 ${WORKDIR}/usb-mount.sh ${D}${bindir}/usb-mount.sh
    install -d ${D}${systemd_unitdir}/system
    install -m 644 ${WORKDIR}/usb-mount@.service ${D}${systemd_unitdir}/system
    install -D -m 0644 ${WORKDIR}/88-usb-mount.rules ${D}${sysconfdir}/udev/rules.d/88-usb-mount.rules
}

FILES:${PN} += " \
	${bindir}/usb-mount.sh \
	${sysconfdir}/udev/rules.d/88-usb-mount.rules \
	${systemd_unitdir}/system/usb-mount@.service \
"
