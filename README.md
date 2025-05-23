# Yocto Docker Monitoring Suite

A comprehensive Embedded Linux project that combines a custom Yocto image with Docker support, deploys Home Assistant via container, and monitors system resources using a C++ application with Telegram bot integration.

## 📌 Project Overview

This suite is designed for embedded system developers and trainers aiming to build secure, customizable, and monitorable IoT platforms using the Yocto Project. The project includes:

- 🛠️ **Custom Yocto Image** with Docker Engine integration
- 🏠 **Home Assistant** deployment via Docker
- 📊 **System Resource Monitor** in C++ (CPU, Memory, Disk)
- 📩 **Telegram Bot Integration** for periodic metric reporting
- 🔁 **Systemd Services** to auto-start components at boot

---

## 📋 Requirements

### ✅ Host System
- Ubuntu 20.04+ or Debian-based distro
- At least 100 GB free disk space
- 8+ GB RAM recommended

### ✅ Packages
Install required dependencies:
```bash
sudo apt update && sudo apt install \
  gawk wget git-core diffstat unzip texinfo gcc-multilib \
  build-essential chrpath socat cpio python3 python3-pip \
  python3-pexpect xz-utils debianutils iputils-ping \
  libsdl1.2-dev xterm docker.io curl cmake
