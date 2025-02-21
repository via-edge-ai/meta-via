DESCRIPTION = "LTE APN parser"

LICENSE = "CLOSED"

FILESPATH =+ "${WORKSPACE}:"

SRC_URI = " \
	file://lte-apn \
	file://apns-conf.xml \
"
S = "${WORKDIR}/lte-apn"

DEPENDS = "libxml2"
CFLAGS += "-I${STAGING_INCDIR}/libxml2"
LDFLAGS += "-lxml2"
TARGET_CC_ARCH += "${LDFLAGS}"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/lte-apn ${D}${bindir}
	install -D -m 0644 ${WORKDIR}/apns-conf.xml ${D}${sysconfdir}/apns-conf.xml
}

FILES_${PN} += " \
	${bindir}/lte-apn \
	${sysconfdir}/apns-conf.xml \
"

#INSANE_SKIP_${PN} += "ldflags"
