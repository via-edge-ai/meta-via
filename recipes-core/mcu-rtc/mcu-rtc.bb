DESCRIPTION = "MCU rtc sync system clock service"
LICENSE = "CLOSED"
inherit systemd
SRC_URI = " \
	file://mcu_rtc_sync.service \
	file://rtc_sync.sh \
"

#RDEPENDS:${PN} += "bash"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'mcu_rtc_sync.service', '', d)}"

do_install() {
     install -D -m 0755 ${WORKDIR}/rtc_sync.sh ${D}${bindir}/rtc_sync.sh
     install -d ${D}${systemd_unitdir}/system
     install -m 644 ${WORKDIR}/mcu_rtc_sync.service ${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
	${bindir}/rtc_sync.sh \
	${systemd_unitdir}/system/mcu_rtc_sync.service \
"
