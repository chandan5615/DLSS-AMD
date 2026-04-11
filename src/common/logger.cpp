#include "logger.h"
#include <iostream>
#include <iomanip>

namespace afglobal {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::Initialize(const std::string& logPath, LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    log_file_.open(logPath, std::ios::app);
    current_level_ = level;
    initialized_ = true;
    
    if (log_file_.is_open()) {
        LogMessage(LogLevel::INFO, "Logger initialized");
    }
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_level_ = level;
}

void Logger::Debug(const std::string& message) {
    if (current_level_ <= LogLevel::DEBUG) {
        LogMessage(LogLevel::DEBUG, message);
    }
}

void Logger::Info(const std::string& message) {
    if (current_level_ <= LogLevel::INFO) {
        LogMessage(LogLevel::INFO, message);
    }
}

void Logger::Warning(const std::string& message) {
    if (current_level_ <= LogLevel::WARNING) {
        LogMessage(LogLevel::WARNING, message);
    }
}

void Logger::Error(const std::string& message) {
    if (current_level_ <= LogLevel::ERR) {
        LogMessage(LogLevel::ERR, message);
    }
}

void Logger::Critical(const std::string& message) {
    if (current_level_ <= LogLevel::CRITICAL) {
        LogMessage(LogLevel::CRITICAL, message);
    }
}

void Logger::LogMessage(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string logLine = "[" + GetTimestamp() + "] [" + LevelToString(level) + "] " + message;
    
    if (log_file_.is_open()) {
        log_file_ << logLine << "\n";
        log_file_.flush();
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

}  // namespace afglobal
