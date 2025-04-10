DESCRIPTION = "A Gstreamer Plug-in for VIA LenZ Plug-ins with USB Cameras."
LICENSE = "CLOSED"

SRC_URI = "\
	file://liblenz.so \
"

S = "${WORKDIR}"

RDEPENDS:${PN} = " gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad gstreamer1.0-plugins-good "

do_install() {
	install -D -m 644 ${WORKDIR}/liblenz.so ${D}/usr/lib/gstreamer-1.0/liblenz.so
}


INSANE_SKIP:${PN} = "file-rdeps already-stripped"
FILES:${PN} = " /usr/lib "

