#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <iomanip>

using json = nlohmann::json;
namespace fs = boost::filesystem;

class SystemMonitor {
private:
    std::string telegram_token;
    std::string chat_id;
    const int INTERVAL = 1800; // 30 minutes in seconds (change to 60 for testing)

    // Callback function for CURL to handle response
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // Load configuration from JSON file
    bool loadConfig() {
        try {
            std::ifstream config_file("/etc/system-monitor/config.json");
            if (!config_file.is_open()) {
                std::cerr << "Error: Cannot open config file /etc/system-monitor/config.json" << std::endl;
                return false;
            }

            json config;
            config_file >> config;

            telegram_token = config["telegram_token"];
            chat_id = config["chat_id"];

            std::cout << "Configuration loaded successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading configuration: " << e.what() << std::endl;
            return false;
        }
    }

    // Get CPU usage percentage
    double getCPUUsage() {
        static unsigned long long lastTotalUser = 0, lastTotalUserLow = 0, lastTotalSys = 0, lastTotalIdle = 0;
        
        std::ifstream file("/proc/stat");
        std::string line;
        std::getline(file, line);
        
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

    // Get memory usage in MB
    std::pair<long, long> getMemoryUsage() {
        std::ifstream file("/proc/meminfo");
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
        
        // Convert from KB to MB
        memTotal /= 1024;
        long memUsed = memTotal - (memAvailable / 1024);
        
        return std::make_pair(memUsed, memTotal);
    }

    // Get disk usage for root filesystem
    std::pair<std::string, std::string> getDiskUsage() {
        FILE* fp = popen("df -h / | tail -1", "r");
        if (fp == nullptr) {
            return std::make_pair("N/A", "N/A");
        }
        
        char buffer[256];
        std::string result;
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            result += buffer;
        }
        pclose(fp);
        
        std::istringstream iss(result);
        std::string filesystem, size, used, available, percent, mountpoint;
        iss >> filesystem >> size >> used >> available >> percent >> mountpoint;
        
        return std::make_pair(used, size);
    }

    // Get current timestamp
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // URL encode string for Telegram API
    std::string urlEncode(const std::string& str) {
        CURL* curl = curl_easy_init();
        if (curl) {
            char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
            if (encoded) {
                std::string result(encoded);
                curl_free(encoded);
                curl_easy_cleanup(curl);
                return result;
            }
            curl_easy_cleanup(curl);
        }
        return str;
    }

    // Send message via Telegram Bot API
    bool sendTelegramMessage(const std::string& message) {
        CURL* curl;
        CURLcode res;
        std::string response;

        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL" << std::endl;
            return false;
        }

        std::string url = "https://api.telegram.org/bot" + telegram_token + "/sendMessage";
        std::string postData = "chat_id=" + chat_id + "&text=" + urlEncode(message) + "&parse_mode=HTML";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "CURL failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        std::cout << "Message sent successfully: " << response << std::endl;
        return true;
    }

public:
    SystemMonitor() = default;

    bool initialize() {
        // Initialize CURL globally
        curl_global_init(CURL_GLOBAL_DEFAULT);
        
        // Load configuration
        return loadConfig();
    }

    void cleanup() {
        curl_global_cleanup();
    }

    void run() {
        std::cout << "System Monitor started. Monitoring interval: " << INTERVAL << " seconds" << std::endl;
        
        while (true) {
            try {
                // Collect system metrics
                double cpuUsage = getCPUUsage();
                auto memUsage = getMemoryUsage();
                auto diskUsage = getDiskUsage();
                std::string timestamp = getCurrentTimestamp();

                // Create formatted message
                std::stringstream message;
                message << "🖥️ <b>System Metrics Report</b>\n";
                message << "-------------------\n";
                message << "📊 <b>CPU Usage:</b> " << std::fixed << std::setprecision(1) << cpuUsage << "%\n";
                message << "🧠 <b>Memory Usage:</b> " << memUsage.first << "MB / " << memUsage.second << "MB\n";
                message << "💾 <b>Disk Usage:</b> " << diskUsage.first << " / " << diskUsage.second << "\n";
                message << "📡 <b>Network:</b> Connected\n";
                message << "⏰ <b>Timestamp:</b> " << timestamp << "\n";

                std::string messageStr = message.str();
                std::cout << "Sending system metrics..." << std::endl;
                std::cout << messageStr << std::endl;

                // Send to Telegram
                if (!sendTelegramMessage(messageStr)) {
                    std::cerr << "Failed to send Telegram message" << std::endl;
                }

            } catch (const std::exception& e) {
                std::cerr << "Error collecting metrics: " << e.what() << std::endl;
            }

            // Wait for next interval
            std::this_thread::sleep_for(std::chrono::seconds(INTERVAL));
        }
    }
};

int main() {
    std::cout << "Starting System Monitor Application..." << std::endl;

    SystemMonitor monitor;
    
    if (!monitor.initialize()) {
        std::cerr << "Failed to initialize system monitor" << std::endl;
        return 1;
    }

    // Set up signal handling for graceful shutdown
    // (In a production system, you'd want proper signal handling)
    
    try {
        monitor.run();
    } catch (const std::exception& e) {
        std::cerr << "System monitor error: " << e.what() << std::endl;
        monitor.cleanup();
        return 1;
    }

    monitor.cleanup();
    return 0;
} 