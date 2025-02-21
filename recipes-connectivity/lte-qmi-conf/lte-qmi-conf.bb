DESCRIPTION = "Init LTE module with qmi-conf scripts"

LICENSE = "CLOSED"

inherit systemd

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = " \
	file://qmi-network.conf \
	file://init.telit.qmi.sh \
	file://set_telit_atcmd.sh \
	file://init_telit_qmi.service \
"
S = "${WORKDIR}/lte-qmi-conf"

RDEPENDS:${PN} += "libqmi lte-apn bash"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'init_telit_qmi.service', '', d)}"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
	install -D -m 0644 ${WORKDIR}/qmi-network.conf ${D}${sysconfdir}/qmi-network.conf
	install -D -m 0755 ${WORKDIR}/init.telit.qmi.sh ${D}${bindir}/init.telit.qmi.sh
	install -D -m 0755 ${WORKDIR}/set_telit_atcmd.sh ${D}${bindir}/set_telit_atcmd.sh
	install -D -m 0644 ${WORKDIR}/init_telit_qmi.service ${D}${systemd_unitdir}/system/init_telit_qmi.service
}

FILES_${PN} += " \
	${sysconfdir}/qmi-network.conf \
	${bindir}/init.telit.qmi.sh \
	${bindir}/set_telit_atcmd.sh \
	${systemd_unitdir}/system/init_telit_qmi.service \
"

#INSANE_SKIP_${PN} = "ldflags"
