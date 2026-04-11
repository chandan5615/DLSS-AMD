#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace afglobal {

struct Config {
    bool enabled = true;
    int target_multiplier = 2;
    std::vector<std::string> process_blacklist;
    std::vector<std::string> process_whitelist;
    bool fallback_blend_only = false;
    std::string log_level = "INFO";
    std::string hotkey = "CTRL+SHIFT+F";

    // Derived from config
    static Config CreateDefault();
    static Config LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
};

class ConfigManager {
public:
    static ConfigManager& Instance();
    
    const Config& GetConfig() const;
    void ReloadConfig(const std::string& path);
    void UpdateConfig(const Config& newConfig);
    
    bool IsProcessWhitelisted(const std::string& processName) const;
    bool IsProcessBlacklisted(const std::string& processName) const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    mutable std::mutex config_mutex_;
    Config current_config_;
};

}  // namespace afglobal
