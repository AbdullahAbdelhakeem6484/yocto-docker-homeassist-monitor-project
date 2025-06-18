#include "TelegramBot.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TelegramBot::TelegramBot() : initialized(false) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

TelegramBot::~TelegramBot() {
    curl_global_cleanup();
}

size_t TelegramBot::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string TelegramBot::urlEncode(const std::string& str) {
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

bool TelegramBot::loadConfig(const std::string& configPath) {
    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            lastError = "Cannot open config file: " + configPath;
            return false;
        }

        json config;
        configFile >> config;

        if (!config.contains("telegram_token") || !config.contains("chat_id")) {
            lastError = "Config file missing required fields";
            return false;
        }

        token = config["telegram_token"];
        chatId = config["chat_id"];
        initialized = true;

        std::cout << "Telegram bot configuration loaded" << std::endl;
        return true;
    } catch (const std::exception& e) {
        lastError = "Error loading config: " + std::string(e.what());
        return false;
    }
}

bool TelegramBot::setCredentials(const std::string& botToken, const std::string& chatId) {
    if (botToken.empty() || chatId.empty()) {
        lastError = "Credentials cannot be empty";
        return false;
    }
    
    token = botToken;
    this->chatId = chatId;
    initialized = true;
    return true;
}

bool TelegramBot::sendMessage(const std::string& message) {
    return sendFormattedMessage(message, "HTML");
}

bool TelegramBot::sendFormattedMessage(const std::string& message, const std::string& parseMode) {
    if (!initialized) {
        lastError = "Bot not initialized";
        return false;
    }

    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return false;
    }

    std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";
    std::string postData = "chat_id=" + chatId + 
                          "&text=" + urlEncode(message) + 
                          "&parse_mode=" + parseMode;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        lastError = "CURL failed: " + std::string(curl_easy_strerror(res));
        return false;
    }

    std::cout << "Message sent successfully" << std::endl;
    return true;
}

bool TelegramBot::testConnection() {
    if (!initialized) {
        lastError = "Bot not initialized";
        return false;
    }

    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    std::string url = "https://api.telegram.org/bot" + token + "/getMe";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

std::string TelegramBot::getLastError() const {
    return lastError;
} 