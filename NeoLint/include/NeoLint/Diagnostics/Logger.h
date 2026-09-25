#pragma once

#include <string_view>

namespace neolint::diagnostics
{
enum class Level
{
    Info,
    Warning,
    Error,
};

class Logger
{
public:
    static void Write(Level level, std::string_view message);
    static void Info(std::string_view message);
    static void Warning(std::string_view message);
    static void Error(std::string_view message);
};
}