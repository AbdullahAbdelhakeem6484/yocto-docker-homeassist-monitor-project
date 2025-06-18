SUMMARY = "Docker System Monitor Application"
DESCRIPTION = "Advanced C++ application for monitoring Docker containers and system resources with Telegram bot integration"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://src/main.cpp \
           file://src/SystemMonitor.cpp \
           file://src/SystemMonitor.h \
           file://src/TelegramBot.cpp \
           file://src/TelegramBot.h \
           file://docker-system-monitor.service \
           file://config.json \
          "

S = "${WORKDIR}"

DEPENDS = "libcurl boost nlohmann-json"
RDEPENDS:${PN} = "libcurl boost nlohmann-json docker-ce"

inherit systemd

SYSTEMD_SERVICE:${PN} = "docker-system-monitor.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_compile() {
    ${CXX} ${CXXFLAGS} ${LDFLAGS} \
        -std=c++17 \
        -I${STAGING_INCDIR} \
        -L${STAGING_LIBDIR} \
        -o docker-system-monitor \
        src/main.cpp \
        src/SystemMonitor.cpp \
        src/TelegramBot.cpp \
        -lcurl -lboost_system -lboost_filesystem -pthread
}

do_install() {
    # Install binary
    install -d ${D}${bindir}
    install -m 0755 docker-system-monitor ${D}${bindir}/

    # Install systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 docker-system-monitor.service ${D}${systemd_system_unitdir}/

    # Install configuration directory
    install -d ${D}${sysconfdir}/docker-system-monitor
    install -m 0644 config.json ${D}${sysconfdir}/docker-system-monitor/config.json.example
}

FILES:${PN} += "${systemd_system_unitdir}/docker-system-monitor.service"
FILES:${PN} += "${sysconfdir}/docker-system-monitor/config.json.example" 