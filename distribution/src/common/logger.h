#pragma once

#include "constants.h"
#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>

namespace afglobal {

class Logger {
public:
    static Logger& Instance();

    void Initialize(const std::string& logPath, LogLevel level = LogLevel::INFO);
    void SetLogLevel(LogLevel level);

    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);
    void Critical(const std::string& message);

    template<typename... Args>
    void DebugF(const char* format, Args... args) {
        if (current_level_ <= LogLevel::DEBUG) {
            LogFormatted(LogLevel::DEBUG, format, args...);
        }
    }

    template<typename... Args>
    void InfoF(const char* format, Args... args) {
        if (current_level_ <= LogLevel::INFO) {
            LogFormatted(LogLevel::INFO, format, args...);
        }
    }

    template<typename... Args>
    void WarningF(const char* format, Args... args) {
        if (current_level_ <= LogLevel::WARNING) {
            LogFormatted(LogLevel::WARNING, format, args...);
        }
    }

    template<typename... Args>
    void ErrorF(const char* format, Args... args) {
        if (current_level_ <= LogLevel::ERR) {
            LogFormatted(LogLevel::ERR, format, args...);
        }
    }

    template<typename... Args>
    void CriticalF(const char* format, Args... args) {
        if (current_level_ <= LogLevel::CRITICAL) {
            LogFormatted(LogLevel::CRITICAL, format, args...);
        }
    }

private:
    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void LogMessage(LogLevel level, const std::string& message);
    std::string GetTimestamp();
    std::string LevelToString(LogLevel level);

    template<typename... Args>
    void LogFormatted(LogLevel level, const char* format, Args... args) {
        // Simple implementation - just log the format string
        // In production, use snprintf or similar
        LogMessage(level, format);
    }

    std::mutex log_mutex_;
    std::ofstream log_file_;
    LogLevel current_level_ = LogLevel::INFO;
    bool initialized_ = false;
};

}  // namespace afglobal
