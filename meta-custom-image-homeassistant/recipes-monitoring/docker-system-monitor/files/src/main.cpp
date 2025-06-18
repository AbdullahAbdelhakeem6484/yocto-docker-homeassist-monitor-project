#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <csignal>
#include "SystemMonitor.h"
#include "TelegramBot.h"

// Global flag for graceful shutdown
volatile sig_atomic_t running = 1;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    running = 0;
}

int main() {
    std::cout << "Starting Docker System Monitor Application..." << std::endl;
    std::cout << "================================================" << std::endl;

    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize components
    SystemMonitor monitor;
    TelegramBot bot;

    // Load Telegram bot configuration
    if (!bot.loadConfig("/etc/docker-system-monitor/config.json")) {
        std::cerr << "Failed to load Telegram bot configuration: " << bot.getLastError() << std::endl;
        std::cerr << "Please ensure /etc/docker-system-monitor/config.json exists with:" << std::endl;
        std::cerr << "  {" << std::endl;
        std::cerr << "    \"telegram_token\": \"YOUR_BOT_TOKEN\"," << std::endl;
        std::cerr << "    \"chat_id\": \"YOUR_CHAT_ID\"" << std::endl;
        std::cerr << "  }" << std::endl;
        return 1;
    }

    // Test Telegram connection
    std::cout << "Testing Telegram bot connection..." << std::endl;
    if (!bot.testConnection()) {
        std::cerr << "Warning: Telegram bot connection test failed: " << bot.getLastError() << std::endl;
        std::cerr << "Messages may not be delivered properly." << std::endl;
    } else {
        std::cout << "Telegram bot connection successful!" << std::endl;
        
        // Send startup message
        std::string startupMessage = "🚀 <b>Docker System Monitor Started</b>\n"
                                   "-------------------\n"
                                   "✅ System monitoring active\n"
                                   "🐳 Docker integration enabled\n"
                                   "📡 Telegram notifications ready\n"
                                   "⏱️ Monitoring interval: 30 minutes\n\n"
                                   "The system will now monitor:\n"
                                   "• CPU and Memory usage\n"
                                   "• Disk space utilization\n"
                                   "• Docker container status\n"
                                   "• Container resource usage";
        
        if (!bot.sendMessage(startupMessage)) {
            std::cerr << "Failed to send startup message: " << bot.getLastError() << std::endl;
        }
    }

    // Main monitoring loop
    const int MONITORING_INTERVAL = 1800; // 30 minutes in seconds (change to 60 for testing)
    std::cout << "Starting monitoring loop with " << MONITORING_INTERVAL << " second intervals..." << std::endl;

    int cycleCount = 0;
    while (running) {
        try {
            cycleCount++;
            std::cout << "\n--- Monitoring Cycle #" << cycleCount << " ---" << std::endl;

            // Collect system metrics
            SystemMetrics metrics = monitor.collectMetrics();
            
            // Display metrics locally
            std::cout << "System Metrics Collected:" << std::endl;
            std::cout << "  CPU Usage: " << std::fixed << std::setprecision(1) << metrics.cpuUsage << "%" << std::endl;
            std::cout << "  Memory: " << metrics.memoryUsedMB << "MB / " << metrics.memoryTotalMB << "MB" << std::endl;
            std::cout << "  Disk: " << metrics.diskUsed << " / " << metrics.diskTotal << std::endl;
            std::cout << "  Docker: " << (metrics.dockerRunning ? "Active" : "Inactive") << std::endl;
            std::cout << "  Containers: " << metrics.containerCount << std::endl;

            // Format and send basic metrics message
            std::string metricsMessage = monitor.formatMetricsMessage(metrics);
            std::cout << "Sending metrics to Telegram..." << std::endl;
            
            if (!bot.sendMessage(metricsMessage)) {
                std::cerr << "Failed to send metrics message: " << bot.getLastError() << std::endl;
            }

            // Send detailed Docker report if containers are present
            if (metrics.dockerRunning && metrics.containerCount > 0) {
                std::cout << "Collecting detailed Docker container information..." << std::endl;
                
                // Get detailed container information
                auto containers = monitor.getDockerContainers();
                if (!containers.empty()) {
                    std::string dockerReport = monitor.formatDetailedDockerReport(containers);
                    
                    std::cout << "Sending Docker container report..." << std::endl;
                    if (!bot.sendMessage(dockerReport)) {
                        std::cerr << "Failed to send Docker report: " << bot.getLastError() << std::endl;
                    }
                }
            }

            std::cout << "Monitoring cycle completed successfully." << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error during monitoring cycle: " << e.what() << std::endl;
            
            // Send error notification
            std::string errorMessage = "⚠️ <b>Monitoring Error</b>\n"
                                     "-------------------\n"
                                     "❌ Error: " + std::string(e.what()) + "\n"
                                     "🔄 Monitoring will continue with next cycle";
            
            bot.sendMessage(errorMessage);
        }

        // Wait for next cycle or handle shutdown
        std::cout << "Waiting " << MONITORING_INTERVAL << " seconds until next cycle..." << std::endl;
        
        for (int i = 0; i < MONITORING_INTERVAL && running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Send shutdown message
    std::cout << "Sending shutdown notification..." << std::endl;
    std::string shutdownMessage = "🛑 <b>Docker System Monitor Stopped</b>\n"
                                 "-------------------\n"
                                 "⏹️ Monitoring service has been stopped\n"
                                 "📊 Total monitoring cycles: " + std::to_string(cycleCount) + "\n"
                                 "👋 Goodbye!";
    
    bot.sendMessage(shutdownMessage);

    std::cout << "Docker System Monitor shutdown complete." << std::endl;
    return 0;
} 