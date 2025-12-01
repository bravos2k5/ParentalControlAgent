#include "Logger.h"
#include <windows.h>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::init(const std::string& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return;
    
    m_logFile.open(logFilePath, std::ios::app);
    m_initialized = m_logFile.is_open();
    
    if (m_initialized) {
        m_logFile << "\n========== Application Started ==========\n";
        m_logFile.flush();
    }
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) return;
    
    m_logFile << getCurrentTimestamp() << " [" << levelToString(level) << "] " << message << "\n";
    m_logFile.flush();
}

void Logger::debug(const std::string& message) {
    log(Level::DBG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INF, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WRN, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERR, message);
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DBG: return "DEBUG";
        case Level::INF: return "INFO";
        case Level::WRN: return "WARNING";
        case Level::ERR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
