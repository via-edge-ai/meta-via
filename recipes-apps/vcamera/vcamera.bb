SUMMARY = "A simple script to run camera with MTK-provided gstreamer commands."
LICENSE = "CLOSED"

DEPENDS += "bash"
RDEPENDS:${PN} += "bash"

#inherit update-rc.d

SRC_URI = "\
	file://vcamera.sh \
	"

do_install() {
	install -D -m 755 ${WORKDIR}/vcamera.sh ${D}${bindir}/vcamera.sh
}
