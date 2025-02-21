DESCRIPTION = "Model Benchmark Tool"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://modelmark \
"

INSANE_SKIP:${PN} += "already-stripped"

DEPENDS += " libmali nnstreamer"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/modelmark ${D}${bindir}
}

FILES:${PN} += "${bindir}/modelmark"
