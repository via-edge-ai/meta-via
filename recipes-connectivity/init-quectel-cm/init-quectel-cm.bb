DESCRIPTION = "Initialize Quectel CM on boot time"

LICENSE = "CLOSED"

RDEPENDS:${PN} += "quectelcm lte-apn bash"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = "file://init-quectel-cm.sh \
           file://init_quectel_cm.service \
           file://60dns \
"

inherit systemd

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'init_quectel_cm.service', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install () {
    install -D -m 0755 ${WORKDIR}/init-quectel-cm.sh ${D}${bindir}/init-quectel-cm.sh
    if [ "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'systemd', '', d)}" = "systemd" ]; then
        install -m 0644 ${WORKDIR}/init_quectel_cm.service -D ${D}${systemd_system_unitdir}/init_quectel_cm.service
    fi
    install -D -m 0755 ${WORKDIR}/60dns ${D}/etc/udhcpc.d/60dns
}

FILES:${PN} += "${bindir}/init-quectel-cm.sh \
		${systemd_system_unitdir}/init_quectel_cm.service \
		/etc/udhcpc.d/60dns \
"
