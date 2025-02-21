DESCRIPTION = "mmc auto-mount service"
LICENSE = "CLOSED"
inherit systemd
SRC_URI = " \
	file://88-mmc-mount.rules \
"
#RDEPENDS:${PN} += "bash"
SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "disable"
do_install() {
    install -D -m 0644 ${WORKDIR}/88-mmc-mount.rules ${D}${sysconfdir}/udev/rules.d/88-mmc-mount.rules
}
FILES:${PN} += " \
	${sysconfdir}/udev/rules.d/88-mmc-mount.rules \
"
