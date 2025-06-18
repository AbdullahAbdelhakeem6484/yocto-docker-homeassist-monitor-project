#include "SystemMonitor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include <algorithm>

// Static member initialization
unsigned long long SystemMonitor::lastTotalUser = 0;
unsigned long long SystemMonitor::lastTotalUserLow = 0;
unsigned long long SystemMonitor::lastTotalSys = 0;
unsigned long long SystemMonitor::lastTotalIdle = 0;

SystemMonitor::SystemMonitor() {
    std::cout << "SystemMonitor initialized" << std::endl;
}

SystemMonitor::~SystemMonitor() {
    std::cout << "SystemMonitor destroyed" << std::endl;
}

std::string SystemMonitor::executeCommand(const std::string& command) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

double SystemMonitor::calculateCPUUsage() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return 0.0;
    }
    
    std::string line;
    std::getline(file, line);
    file.close();
    
    std::istringstream iss(line);
    std::string cpu;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;
    
    iss >> cpu >> totalUser >> totalUserLow >> totalSys >> totalIdle;
    
    if (lastTotalUser == 0) {
        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;
        return 0.0;
    }
    
    total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) + (totalSys - lastTotalSys);
    double percent = total;
    total += (totalIdle - lastTotalIdle);
    percent /= total;
    percent *= 100;
    
    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;
    
    return percent;
}

std::pair<long, long> SystemMonitor::getMemoryInfo() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        return std::make_pair(0, 0);
    }
    
    std::string line;
    long memTotal = 0, memAvailable = 0;
    
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label, unit;
            iss >> label >> memTotal >> unit;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line);
            std::string label, unit;
            iss >> label >> memAvailable >> unit;
        }
    }
    file.close();
    
    // Convert from KB to MB
    memTotal /= 1024;
    long memUsed = memTotal - (memAvailable / 1024);
    
    return std::make_pair(memUsed, memTotal);
}

std::pair<std::string, std::string> SystemMonitor::getDiskInfo() {
    std::string result = executeCommand("df -h / | tail -1");
    if (result.empty()) {
        return std::make_pair("N/A", "N/A");
    }
    
    std::istringstream iss(result);
    std::string filesystem, size, used, available, percent, mountpoint;
    iss >> filesystem >> size >> used >> available >> percent >> mountpoint;
    
    return std::make_pair(used, size);
}

std::string SystemMonitor::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool SystemMonitor::isDockerRunning() {
    std::string result = executeCommand("systemctl is-active docker 2>/dev/null");
    return result.find("active") != std::string::npos;
}

std::vector<DockerContainer> SystemMonitor::getDockerContainers() {
    std::vector<DockerContainer> containers;
    
    if (!isDockerRunning()) {
        return containers;
    }
    
    // Get container list
    std::string containerList = executeCommand("docker ps -a --format \"{{.ID}}|{{.Names}}|{{.Image}}|{{.Status}}|{{.CreatedAt}}\" 2>/dev/null");
    
    if (containerList.empty()) {
        return containers;
    }
    
    std::istringstream iss(containerList);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (line.empty()) continue;
        
        DockerContainer container;
        std::istringstream lineStream(line);
        std::string token;
        
        // Parse pipe-separated values
        std::getline(lineStream, container.id, '|');
        std::getline(lineStream, container.name, '|');
        std::getline(lineStream, container.image, '|');
        std::getline(lineStream, container.status, '|');
        std::getline(lineStream, container.created, '|');
        
        // Get stats for running containers
        if (container.status.find("Up") != std::string::npos) {
            std::string statsCmd = "docker stats --no-stream --format \"{{.CPUPerc}}|{{.MemUsage}}|{{.NetIO}}|{{.BlockIO}}\" " + container.id + " 2>/dev/null";
            std::string stats = executeCommand(statsCmd);
            
            if (!stats.empty()) {
                std::istringstream statsStream(stats);
                std::string cpuStr, memStr, netStr, blockStr;
                
                std::getline(statsStream, cpuStr, '|');
                std::getline(statsStream, memStr, '|');
                std::getline(statsStream, netStr, '|');
                std::getline(statsStream, blockStr, '|');
                
                // Parse CPU percentage
                cpuStr.erase(std::remove(cpuStr.begin(), cpuStr.end(), '%'), cpuStr.end());
                try {
                    container.cpuPercent = std::stod(cpuStr);
                } catch (...) {
                    container.cpuPercent = 0.0;
                }
                
                container.memoryUsage = memStr;
                container.networkIO = netStr;
                container.blockIO = blockStr;
            }
        } else {
            container.cpuPercent = 0.0;
            container.memoryUsage = "0B / 0B";
            container.networkIO = "0B / 0B";
            container.blockIO = "0B / 0B";
        }
        
        containers.push_back(container);
    }
    
    return containers;
}

SystemMetrics SystemMonitor::collectMetrics() {
    SystemMetrics metrics;
    
    // Collect system metrics
    metrics.cpuUsage = calculateCPUUsage();
    auto memInfo = getMemoryInfo();
    metrics.memoryUsedMB = memInfo.first;
    metrics.memoryTotalMB = memInfo.second;
    
    auto diskInfo = getDiskInfo();
    metrics.diskUsed = diskInfo.first;
    metrics.diskTotal = diskInfo.second;
    
    metrics.timestamp = getCurrentTimestamp();
    metrics.dockerRunning = isDockerRunning();
    
    // Collect Docker info
    auto containers = getDockerContainers();
    metrics.containerCount = containers.size();
    
    for (const auto& container : containers) {
        metrics.containerNames.push_back(container.name);
    }
    
    return metrics;
}

std::string SystemMonitor::formatMetricsMessage(const SystemMetrics& metrics) {
    std::stringstream message;
    
    message << "🖥️ <b>System Metrics Report</b>\n";
    message << "-------------------\n";
    message << "📊 <b>CPU Usage:</b> " << std::fixed << std::setprecision(1) << metrics.cpuUsage << "%\n";
    message << "🧠 <b>Memory Usage:</b> " << metrics.memoryUsedMB << "MB / " << metrics.memoryTotalMB << "MB\n";
    message << "💾 <b>Disk Usage:</b> " << metrics.diskUsed << " / " << metrics.diskTotal << "\n";
    message << "🐳 <b>Docker Status:</b> " << (metrics.dockerRunning ? "Active" : "Inactive") << "\n";
    message << "📦 <b>Containers:</b> " << metrics.containerCount << " total\n";
    
    if (!metrics.containerNames.empty()) {
        message << "🏷️ <b>Container Names:</b>\n";
        for (size_t i = 0; i < metrics.containerNames.size() && i < 5; ++i) {
            message << "   • " << metrics.containerNames[i] << "\n";
        }
        if (metrics.containerNames.size() > 5) {
            message << "   • ... and " << (metrics.containerNames.size() - 5) << " more\n";
        }
    }
    
    message << "📡 <b>Network:</b> Connected\n";
    message << "⏰ <b>Timestamp:</b> " << metrics.timestamp << "\n";
    
    return message.str();
}

std::string SystemMonitor::formatDetailedDockerReport(const std::vector<DockerContainer>& containers) {
    if (containers.empty()) {
        return "🐳 <b>Docker Container Report</b>\n-------------------\nNo containers found.";
    }
    
    std::stringstream report;
    report << "🐳 <b>Docker Container Report</b>\n";
    report << "-------------------\n";
    report << "📦 <b>Total Containers:</b> " << containers.size() << "\n\n";
    
    for (const auto& container : containers) {
        report << "🏷️ <b>" << container.name << "</b>\n";
        report << "   📋 ID: " << container.id.substr(0, 12) << "\n";
        report << "   🖼️ Image: " << container.image << "\n";
        report << "   ⚡ Status: " << container.status << "\n";
        
        if (container.status.find("Up") != std::string::npos) {
            report << "   💻 CPU: " << std::fixed << std::setprecision(1) << container.cpuPercent << "%\n";
            report << "   🧠 Memory: " << container.memoryUsage << "\n";
            report << "   🌐 Network I/O: " << container.networkIO << "\n";
            report << "   💿 Block I/O: " << container.blockIO << "\n";
        }
        
        report << "   📅 Created: " << container.created << "\n";
        report << "\n";
    }
    
    return report.str();
} 