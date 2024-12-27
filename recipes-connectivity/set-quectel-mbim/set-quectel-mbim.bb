DESCRIPTION = "Check/Set Quectel LTE module to MBIM mode"

LICENSE = "CLOSED"

RDEPENDS:${PN} += "bash"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = "file://set-quectel-mbim.sh \
           file://set_quectel_mbim.service \
"

inherit systemd

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'set_quectel_mbim.service', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install () {
    install -D -m 0755 ${WORKDIR}/set-quectel-mbim.sh ${D}${bindir}/set-quectel-mbim.sh
    if [ "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'systemd', '', d)}" = "systemd" ]; then
        install -m 0644 ${WORKDIR}/set_quectel_mbim.service -D ${D}${systemd_system_unitdir}/set_quectel_mbim.service
    fi
}

FILES:${PN} += "${bindir}/set-quectel-mbim.sh \
		${systemd_system_unitdir}/set_quectel_mbim.service \
"
