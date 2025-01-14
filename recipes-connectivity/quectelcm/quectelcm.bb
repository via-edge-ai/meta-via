DESCRIPTION = "quectelcm"

LICENSE = "CLOSED"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = "file://quectelcm \
		   file://via-set-ip \
"
S = "${WORKDIR}/quectelcm"

do_install() {
       install -d ${D}${bindir}
       install -m 0755 ${S}/quectelcm ${D}${bindir}
	   install -m 0755 ${WORKDIR}/via-set-ip ${D}${bindir}
}

FILES:${PN} += "${bindir}/quectelcm \
		${bindir}/via-set-ip \
"

INSANE_SKIP:${PN} += "ldflags"
