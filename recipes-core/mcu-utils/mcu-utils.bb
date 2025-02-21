DESCRIPTION = "VIA MCU UTILS"
LICENSE = "Closed"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://src"

S = "${WORKDIR}/src"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile() {
    make
}

do_install() {
    install -d ${D}${bindir}
    install -m 755 mcu-utils ${D}${bindir}
}

