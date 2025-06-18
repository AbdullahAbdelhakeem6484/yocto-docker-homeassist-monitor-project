#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <string>
#include <vector>
#include <memory>

struct SystemMetrics {
    double cpuUsage;
    long memoryUsedMB;
    long memoryTotalMB;
    std::string diskUsed;
    std::string diskTotal;
    std::string timestamp;
    bool dockerRunning;
    int containerCount;
    std::vector<std::string> containerNames;
};

struct DockerContainer {
    std::string id;
    std::string name;
    std::string image;
    std::string status;
    std::string created;
    double cpuPercent;
    std::string memoryUsage;
    std::string networkIO;
    std::string blockIO;
};

class SystemMonitor {
private:
    static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
    
    // Private helper methods
    double calculateCPUUsage();
    std::pair<long, long> getMemoryInfo();
    std::pair<std::string, std::string> getDiskInfo();
    std::string getCurrentTimestamp();
    bool isDockerRunning();
    std::vector<DockerContainer> getDockerContainers();
    std::string executeCommand(const std::string& command);

public:
    SystemMonitor();
    ~SystemMonitor();
    
    // Public interface
    SystemMetrics collectMetrics();
    std::string formatMetricsMessage(const SystemMetrics& metrics);
    std::string formatDetailedDockerReport(const std::vector<DockerContainer>& containers);
};

#endif // SYSTEMMONITOR_H 