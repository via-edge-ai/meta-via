DESCRIPTION = "Init LTE module with mbim-conf scripts"

LICENSE = "CLOSED"

inherit systemd

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = " \
	file://mbim-network.conf \
	file://init.quectel.mbim.sh \
	file://set_quectel_atcmd.sh \
	file://init_quectel_mbim.service \
"
S = "${WORKDIR}/lte-mbim-conf"

RDEPENDS:${PN} += "libmbim lte-apn bash"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'init_quectel_mbim.service', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
	install -D -m 0644 ${WORKDIR}/mbim-network.conf ${D}${sysconfdir}/mbim-network.conf
	install -D -m 0755 ${WORKDIR}/init.quectel.mbim.sh ${D}${bindir}/init.quectel.mbim.sh
	install -D -m 0755 ${WORKDIR}/set_quectel_atcmd.sh ${D}${bindir}/set_quectel_atcmd.sh
	install -D -m 0644 ${WORKDIR}/init_quectel_mbim.service ${D}${systemd_unitdir}/system/init_quectel_mbim.service
}

FILES_${PN} += " \
	${sysconfdir}/mbim-network.conf \
	${bindir}/init.quectel.mbim.sh \
	${bindir}/set_quectel_atcmd.sh \
	${systemd_unitdir}/system/init_quectel_mbim.service \
"

#INSANE_SKIP_${PN} = "ldflags"
