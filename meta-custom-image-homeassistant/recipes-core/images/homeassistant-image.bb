SUMMARY = "Home Assistant IoT Image with Docker and System Monitoring"
DESCRIPTION = "Custom embedded Linux image with Docker Engine, Home Assistant container support, and real-time system monitoring with Telegram bot integration"

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

# Docker and container support
IMAGE_INSTALL:append = " \
    docker-ce \
    containerd \
    runc \
    iptables \
    bridge-utils \
    iproute2 \
    python3 \
    python3-pip \
    curl \
    wget \
    openssh \
    openssh-sftp-server \
    system-monitor \
    docker-system-monitor \
"

# Network and system utilities
IMAGE_INSTALL:append = " \
    dhcpcd \
    ethtool \
    iperf3 \
    tcpdump \
    net-tools \
    procps \
    htop \
    nano \
    vim \
"

# Development and debugging tools
IMAGE_INSTALL:append = " \
    gdb \
    strace \
    ltrace \
    lsof \
    file \
    which \
    tree \
"

# Additional features
DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED = "sysvinit"
VIRTUAL-RUNTIME_initscripts = ""

# Enable Docker features
DISTRO_FEATURES:append = " virtualization"

# Set root password for development (change in production)
EXTRA_USERS_PARAMS = "usermod -P root root;"

# Image size and format
IMAGE_ROOTFS_EXTRA_SPACE = "2048000"
IMAGE_FSTYPES = "ext4 tar.bz2 wic wic.bz2" 