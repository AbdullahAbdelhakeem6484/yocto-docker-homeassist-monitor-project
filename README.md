# 🏠 Yocto Docker Home Assistant Monitoring Suite

[![Embedded Linux](https://img.shields.io/badge/Platform-Embedded%20Linux-green.svg)](https://www.yoctoproject.org/)
[![Docker](https://img.shields.io/badge/Container-Docker-blue.svg)](https://www.docker.com/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-red.svg)](https://isocpp.org/)
[![Telegram](https://img.shields.io/badge/Integration-Telegram%20Bot-blue.svg)](https://core.telegram.org/bots)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A comprehensive **Embedded Linux** project demonstrating advanced **Yocto Project** development skills by creating a custom Linux image with **Docker Engine** integration, **Home Assistant** container deployment, and real-time **system monitoring** with **Telegram bot** notifications.

## 🎯 Project Overview

This repository contains the **custom meta layer** and configuration for building an embedded Linux system that combines:

- **🛠️ Custom Yocto Image Building** - Creating a minimal yet feature-rich embedded Linux distribution
- **🐳 Container Orchestration** - Docker Engine integration for embedded environments  
- **🏠 IoT Platform Deployment** - Home Assistant as a containerized smart home hub
- **📊 Real-time System Monitoring** - C++ applications for resource tracking
- **🤖 Automated Notifications** - Telegram bot integration for remote monitoring
- **⚙️ Service Management** - Systemd integration for reliability and auto-startup

## 📹 Video Demonstration

**YouTube Demo:** [Yocto Docker Home Assistant Monitor - Complete Walkthrough](https://youtu.be/Oh9E_T-yPNs)

## 🏗️ Repository Contents

This repository contains **only the custom components** you need to build the project:

```
meta-custom-image-homeassistant/           # Custom meta layer (YOUR WORK)
├── conf/layer.conf                        # Layer configuration
├── recipes-core/images/
│   └── homeassistant-image.bb            # Custom image recipe
└── recipes-monitoring/
    ├── system-monitor/                    # Core system monitor
    │   ├── files/src/main.cpp            # C++ monitoring application
    │   ├── files/system-monitor.service  # Systemd service
    │   └── system-monitor_1.0.bb         # Recipe
    └── docker-system-monitor/             # Docker-specific monitor
        ├── files/src/
        │   ├── main.cpp                  # Main application
        │   ├── SystemMonitor.cpp         # System metrics collection
        │   └── TelegramBot.cpp           # Telegram integration
        ├── files/docker-system-monitor.service
        └── docker-system-monitor_1.0.bb
```

## 🚀 Quick Start

### Prerequisites

**Host System Requirements:**
- Ubuntu 20.04+ or Debian-based distribution
- Minimum 100 GB free disk space
- 8+ GB RAM recommended
- Internet connection for downloading sources

**Install Dependencies:**
```bash
sudo apt update && sudo apt install \
  gawk wget git-core diffstat unzip texinfo gcc-multilib \
  build-essential chrpath socat cpio python3 python3-pip \
  python3-pexpect xz-utils debianutils iputils-ping \
  libsdl1.2-dev xterm docker.io curl cmake
```

### Build Instructions

1. **Download Yocto Poky**
```bash
git clone -b kirkstone https://git.yoctoproject.org/git/poky
cd poky
```

2. **Clone This Repository**
```bash
git clone https://github.com/AbdullahAbdelhakeem6484/yocto-docker-homeassist-monitor-project.git
```

3. **Get Additional Meta Layers**
```bash
git clone -b kirkstone https://git.openembedded.org/meta-openembedded
git clone -b kirkstone https://git.yoctoproject.org/git/meta-virtualization
```

4. **Setup Build Environment**
```bash
source oe-init-build-env
```

5. **Add Layers**
```bash
bitbake-layers add-layer ../yocto-docker-homeassist-monitor-project/meta-custom-image-homeassistant
bitbake-layers add-layer ../meta-openembedded/meta-oe
bitbake-layers add-layer ../meta-openembedded/meta-python
bitbake-layers add-layer ../meta-openembedded/meta-networking
bitbake-layers add-layer ../meta-virtualization
```

6. **Configure Build**
```bash
echo 'DISTRO_FEATURES:append = " virtualization seccomp ipv4"' >> conf/local.conf
echo 'IMAGE_INSTALL:append = " kernel-modules"' >> conf/local.conf
```

7. **Build Custom Image**
```bash
bitbake homeassistant-image
```

## 🚀 Deployment

### QEMU Testing
```bash
runqemu homeassistant-image
```

### Configure Telegram Bot
```bash
# Create bot configuration on target
sudo mkdir -p /etc/system-monitor
sudo tee /etc/system-monitor/config.json << EOF
{
    "telegram_bot_token": "YOUR_BOT_TOKEN",
    "chat_id": "YOUR_CHAT_ID"
}
EOF
```

### Start Services
```bash
sudo systemctl enable --now system-monitor
sudo systemctl enable --now docker-system-monitor
sudo systemctl enable --now docker
```

## 📊 Features

### ✅ Custom Yocto Image
- **Minimal footprint** optimized for embedded systems
- **Docker Engine** integration with full container support
- **Network utilities** (iptables, iproute2, bridge-utils)
- **SSH access** for remote management
- **Python 3** runtime for Home Assistant dependencies

### ✅ Real-time System Monitoring
- **CPU utilization** tracking via `/proc/stat`
- **Memory consumption** monitoring (used/free/cached)
- **Disk usage** analysis with `df` integration
- **Docker container** resource tracking
- **30-minute interval** reporting (configurable)

### ✅ Telegram Bot Integration
- **Real-time notifications** of system metrics
- **Remote monitoring** capabilities
- **JSON configuration** for bot credentials
- **Error handling** and retry logic

### ✅ Service Reliability
- **Systemd integration** for all components
- **Auto-restart** on failure
- **Dependency management** (network-online.target)

## 🛡️ Why This Approach?

This repository contains **only your custom work** rather than the entire Yocto source tree because:

- **Lightweight:** 582MB vs 9.3GB+ for full Yocto
- **Focus:** Highlights your actual contributions
- **Standard Practice:** Industry standard for Yocto projects
- **Reusable:** Others can integrate your layer into their Yocto builds
- **Version Control:** Easier to track changes to your custom code

## 🔧 Technology Stack

| Component | Technology | Purpose |
|-----------|------------|---------|
| **Base OS** | Yocto Project (Kirkstone) | Custom embedded Linux distribution |
| **Containerization** | Docker CE, containerd, runc | Container runtime environment |
| **IoT Platform** | Home Assistant | Smart home automation hub |
| **Monitoring Backend** | C++17, Boost, nlohmann/json | System resource monitoring |
| **Communication** | Telegram Bot API, libcurl | Remote notifications |
| **Service Management** | systemd | Process lifecycle management |
| **Build System** | BitBake, OpenEmbedded | Cross-compilation and packaging |

## 🛠️ Troubleshooting

### Common Build Issues
```bash
# Clean build cache
bitbake -c cleansstate homeassistant-image

# Check layer configuration
bitbake-layers show-layers

# Validate recipes
bitbake -e homeassistant-image | grep IMAGE_INSTALL
```

### Runtime Issues
```bash
# Check service status
systemctl status system-monitor docker-system-monitor

# View logs
journalctl -u system-monitor -f
journalctl -u docker-system-monitor -f

# Container debugging
docker ps -a
docker logs homeassistant
```

## 📈 Sample Output

### Telegram Notifications
```
🖥️ System Metrics Report
-------------------
📊 CPU Usage: 15.2%
🧠 Memory Usage: 245MB / 1024MB  
💾 Disk Usage: 2.1G / 7.3G (29%)
🐳 Docker Status: Active
📡 Network: Connected
⏰ Timestamp: 2024-12-19 10:30:00
```

## 🤝 Contributing

Contributions are welcome! Please:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** changes (`git commit -m 'Add amazing feature'`)
4. **Push** to branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Abdullah Abdelhakeem**
- **GitHub:** [@AbdullahAbdelhakeem6484](https://github.com/AbdullahAbdelhakeem6484)
- **Email:** abdullah.abdelhakeem657@gmail.com
- **LinkedIn:** [Abdullah Abdelhakeem](https://linkedin.com/in/abdullah-abdelhakeem)

## 🙏 Acknowledgments

- **Yocto Project** community for excellent documentation
- **OpenEmbedded** project for the meta layers
- **Docker Inc.** for containerization technology
- **Home Assistant** team for the IoT platform
- **Telegram** for the Bot API

## 📚 References

- [Yocto Project Documentation](https://docs.yoctoproject.org/)
- [Docker Documentation](https://docs.docker.com/)
- [Home Assistant Documentation](https://www.home-assistant.io/docs/)
- [Telegram Bot API](https://core.telegram.org/bots/api)

---

**⭐ If this project helped you, please give it a star! ⭐** 