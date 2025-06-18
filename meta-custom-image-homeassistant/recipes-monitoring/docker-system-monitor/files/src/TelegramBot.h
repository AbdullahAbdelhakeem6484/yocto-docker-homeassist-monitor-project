#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <string>
#include <curl/curl.h>

class TelegramBot {
private:
    std::string token;
    std::string chatId;
    
    // Callback function for CURL
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    
    // URL encode string for API calls
    std::string urlEncode(const std::string& str);

public:
    TelegramBot();
    ~TelegramBot();
    
    // Configuration
    bool loadConfig(const std::string& configPath);
    bool setCredentials(const std::string& botToken, const std::string& chatId);
    
    // Message sending
    bool sendMessage(const std::string& message);
    bool sendFormattedMessage(const std::string& message, const std::string& parseMode = "HTML");
    
    // Utility methods
    bool testConnection();
    std::string getLastError() const;
    
private:
    std::string lastError;
    bool initialized;
};

#endif // TELEGRAMBOT_H 