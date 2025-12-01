#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <sstream>
#include <iomanip>

class Logger {
public:
    enum class Level {
        DBG,
        INF,
        WRN,
        ERR
    };

    static Logger& getInstance();
    
    void init(const std::string& logFilePath);
    void log(Level level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string levelToString(Level level);
    std::string getCurrentTimestamp();

    std::ofstream m_logFile;
    std::mutex m_mutex;
    bool m_initialized = false;
};

#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
