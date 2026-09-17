#include <NeoPM/Utils/Logger.h>
#include <iostream>

namespace NeoPM::Utils {
    Logger& Logger::Instance() {
        static Logger instance;
        return instance;
    }

    void Logger::SetMinLevel(LogLevel level) {
        m_minLevel = level;
    }

    void Logger::Error(const char* category, const std::string& message) {
        if (LogLevel::Error <= m_minLevel) {
            std::cerr << "[ERROR] [" << category << "] " << message << std::endl;
        }
    }

    void Logger::Warning(const char* category, const std::string& message) {
        if (LogLevel::Warning <= m_minLevel) {
            std::cout << "[WARN] [" << category << "] " << message << std::endl;
        }
    }

    void Logger::Info(const char* category, const std::string& message) {
        if (LogLevel::Info <= m_minLevel) {
            std::cout << "[INFO] [" << category << "] " << message << std::endl;
        }
    }

    void Logger::Debug(const char* category, const std::string& message) {
        if (LogLevel::Debug <= m_minLevel) {
            std::cout << "[DEBUG] [" << category << "] " << message << std::endl;
        }
    }
}
