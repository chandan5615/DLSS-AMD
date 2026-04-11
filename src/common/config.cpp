#include "config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

namespace afglobal {

Config Config::CreateDefault() {
    Config config;
    config.enabled = true;
    config.target_multiplier = 2;
    config.process_blacklist = {};
    config.process_whitelist = {};
    config.fallback_blend_only = false;
    config.log_level = "INFO";
    config.hotkey = "CTRL+SHIFT+F";
    return config;
}

Config Config::LoadFromFile(const std::string& path) {
    Config config = CreateDefault();
    
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return config;  // Return default if file doesn't exist
        }
        
        json j;
        file >> j;
        
        if (j.contains("enabled")) config.enabled = j["enabled"];
        if (j.contains("target_multiplier")) {
            int mult = j["target_multiplier"];
            if (mult >= 2 && mult <= 4) {
                config.target_multiplier = mult;
            }
        }
        if (j.contains("process_blacklist") && j["process_blacklist"].is_array()) {
            config.process_blacklist = j["process_blacklist"].get<std::vector<std::string>>();
        }
        if (j.contains("process_whitelist") && j["process_whitelist"].is_array()) {
            config.process_whitelist = j["process_whitelist"].get<std::vector<std::string>>();
        }
        if (j.contains("fallback_blend_only")) config.fallback_blend_only = j["fallback_blend_only"];
        if (j.contains("log_level")) config.log_level = j["log_level"];
        if (j.contains("hotkey")) config.hotkey = j["hotkey"];
    }
    catch (...) {
        // Return default on any error
    }
    
    return config;
}

bool Config::SaveToFile(const std::string& path) const {
    try {
        json j;
        j["enabled"] = enabled;
        j["target_multiplier"] = target_multiplier;
        j["process_blacklist"] = process_blacklist;
        j["process_whitelist"] = process_whitelist;
        j["fallback_blend_only"] = fallback_blend_only;
        j["log_level"] = log_level;
        j["hotkey"] = hotkey;
        
        std::ofstream file(path);
        file << j.dump(2);
        file.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

ConfigManager& ConfigManager::Instance() {
    static ConfigManager instance;
    return instance;
}

const Config& ConfigManager::GetConfig() const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return current_config_;
}

void ConfigManager::ReloadConfig(const std::string& path) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    current_config_ = Config::LoadFromFile(path);
}

void ConfigManager::UpdateConfig(const Config& newConfig) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    current_config_ = newConfig;
}

bool ConfigManager::IsProcessWhitelisted(const std::string& processName) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    if (current_config_.process_whitelist.empty()) {
        return true;  // No whitelist means all are allowed
    }
    return std::find(current_config_.process_whitelist.begin(),
                     current_config_.process_whitelist.end(),
                     processName) != current_config_.process_whitelist.end();
}

bool ConfigManager::IsProcessBlacklisted(const std::string& processName) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return std::find(current_config_.process_blacklist.begin(),
                     current_config_.process_blacklist.end(),
                     processName) != current_config_.process_blacklist.end();
}

}  // namespace afglobal
