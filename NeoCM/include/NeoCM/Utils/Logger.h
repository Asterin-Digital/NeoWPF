/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>

namespace NeoCM::Utils {

class Logger {
public:
    static Logger& Instance();

    void Info(const std::string& message) const;
    void Warning(const std::string& message) const;
    void Error(const std::string& message) const;
    void PrintUsage() const;

private:
    Logger() = default;
    ~Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

}
