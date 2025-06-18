SUMMARY = "System Monitor Application"
DESCRIPTION = "C++ application for monitoring system resources (CPU, memory, disk) with Telegram bot integration"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://src/main.cpp \
           file://system-monitor.service \
           file://src/config.json \
          "

S = "${WORKDIR}"

DEPENDS = "libcurl boost nlohmann-json"
RDEPENDS:${PN} = "libcurl boost nlohmann-json"

inherit systemd

SYSTEMD_SERVICE:${PN} = "system-monitor.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        -std=c++17 \
        -I${STAGING_INCDIR} \
        -L${STAGING_LIBDIR} \
        -o system-monitor \
        src/main.cpp \
        -lcurl -lboost_system -lboost_filesystem -pthread
}

do_install() {
    # Install binary
    install -d ${D}${bindir}
    install -m 0755 system-monitor ${D}${bindir}/

    # Install systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 system-monitor.service ${D}${systemd_system_unitdir}/

    # Install configuration directory
    install -d ${D}${sysconfdir}/system-monitor
    install -m 0644 src/config.json ${D}${sysconfdir}/system-monitor/config.json.example
}

FILES:${PN} += "${systemd_system_unitdir}/system-monitor.service"
FILES:${PN} += "${sysconfdir}/system-monitor/config.json.example" 