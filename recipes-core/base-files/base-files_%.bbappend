FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"

SRC_URI:append = "\
	file://fstab.via \
"

do_install:append() {
	install -D -m 644 ${WORKDIR}/fstab.via ${D}/etc/fstab
}

