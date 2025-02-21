DESCRIPTION = "VIA Thermal"
LICENSE = "Closed"

inherit systemd

LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

TARGET_CC_ARCH += "${LDFLAGS}"

INSANE_SKIP_${PN} += "ldflags"

SRC_URI = " \
	file://vthermal \
	file://vthermal.sh \
	file://vthermal.service \
	file://LICENSE \
"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', '${PN}', '', d)}"
SYSTEMD_SERVICE:${PN} = "${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'vthermal.service', '', d)}"

S = "${WORKDIR}"

do_install() {
   install -d ${D}${bindir}
   install -d ${D}${systemd_unitdir}/system

   install -m 0755 ${S}/vthermal ${D}${bindir}
   install -m 0755 ${S}/vthermal.sh ${D}${bindir}
   install -m 0755 ${S}/vthermal.service ${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
	${bindir}/vthermal \
	${bindir}/vthermal.sh \
	${systemd_unitdir}/system/vthermal.service \
"

