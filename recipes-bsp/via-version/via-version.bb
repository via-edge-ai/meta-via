DESCRIPTION = "VIA Version"
LICENSE = "CLOSED"

SRC_URI = "file://via-release"

do_install() {
    install -d ${D}/etc/
    install -D -m 0755 ${WORKDIR}/via-release ${D}/etc/via-release
}

