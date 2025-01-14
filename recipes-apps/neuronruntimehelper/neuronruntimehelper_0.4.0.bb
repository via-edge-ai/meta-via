DESCRIPTION = "Neuron Runtime Helper"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://NeuronRuntimeHelper-${PV}-cp310-cp310-linux_aarch64.zip \
    file://image_classification \
    file://object_detection \
"

inherit python3-dir

INSANE_SKIP:${PN} += "already-stripped"

do_unpack[depends] += "unzip-native:do_populate_sysroot"

DEPENDS += " \
    python3 \
"

S = "${WORKDIR}"

FILES:${PN} += "\
    ${libdir}/${PYTHON_DIR}/site-packages/ \
    ${libdir}/${PYTHON_DIR}/site-packages/NeuronRuntimeHelper-${PV}.dist-info \
    /usr/share/neuron-runtime-helper/ \
"

do_install() {
    install -d ${D}${libdir}/${PYTHON_DIR}/site-packages/NeuronRuntimeHelper-${PV}.dist-info
    
    install -m 644 ${S}/*.so ${D}${libdir}/${PYTHON_DIR}/site-packages/
    install -m 644 ${S}/NeuronRuntimeHelper-${PV}.dist-info/* ${D}${libdir}/${PYTHON_DIR}/site-packages/NeuronRuntimeHelper-${PV}.dist-info/

    install -d ${D}/usr/share/neuron-runtime-helper/
    install -d ${D}/usr/share/neuron-runtime-helper/image_classification
    install -d ${D}/usr/share/neuron-runtime-helper/object_detection
    install -m 755 ${S}/image_classification/* ${D}/usr/share/neuron-runtime-helper/image_classification/
    install -m 755 ${S}/object_detection/* ${D}/usr/share/neuron-runtime-helper/object_detection/
}
