#include <NeoLint/Diagnostics/Logger.h>

#include <iostream>

namespace neolint::diagnostics
{
void Logger::Write(Level level, std::string_view message)
{
    auto& stream = level == Level::Info ? std::cout : std::cerr;
    const char* label = level == Level::Info ? "info" :
        level == Level::Warning ? "warning" : "error";
    stream << "NeoLint: " << label << ": " << message << '\n' << std::flush;
}

void Logger::Info(std::string_view message)
{
    Write(Level::Info, message);
}

void Logger::Warning(std::string_view message)
{
    Write(Level::Warning, message);
}

void Logger::Error(std::string_view message)
{
    Write(Level::Error, message);
}
}
