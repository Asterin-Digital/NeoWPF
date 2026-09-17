#pragma once

#include <string>

namespace NeoPM::Utils {
    enum class LogLevel {
        Error,
        Warning,
        Info,
        Debug
    };

    class Logger {
    public:
        static Logger& Instance();

        void SetMinLevel(LogLevel level);

        void Error(const char* category, const std::string& message);
        void Warning(const char* category, const std::string& message);
        void Info(const char* category, const std::string& message);
        void Debug(const char* category, const std::string& message);

    private:
        LogLevel m_minLevel = LogLevel::Info;

        Logger() = default;
        ~Logger() = default;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
    };
}
