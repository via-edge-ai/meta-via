DESCRIPTION = "Start MCU WDT"
LICENSE = "CLOSED"

inherit systemd

SRC_URI = " \
    file://start-wdt.sh \
    file://start-wdt.service \
"

#RDEPENDS:${PN} += "bash"

SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'start-wdt.service', '', d)}"

do_install() {
    install -D -m 0755 ${WORKDIR}/start-wdt.sh ${D}${bindir}/start-wdt.sh
    install -d ${D}${systemd_unitdir}/system
    install -m 644 ${WORKDIR}/start-wdt.service ${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
    ${bindir}/start-wdt.sh \
    ${systemd_unitdir}/system/start-wdt.service \
"
