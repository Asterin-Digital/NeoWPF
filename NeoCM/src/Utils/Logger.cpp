#include <NeoCM/Utils/Logger.h>

#include <iostream>

namespace NeoCM::Utils {

Logger& Logger::Instance()
{
    static Logger logger;
    return logger;
}

void Logger::Info(const std::string& message) const
{
    std::cout << "[NeoCM] " << message << '\n';
}

void Logger::Warning(const std::string& message) const
{
    std::cout << "[NeoCM:warning] " << message << '\n';
}

void Logger::Error(const std::string& message) const
{
    std::cerr << "[NeoCM:error] " << message << '\n';
}

void Logger::PrintUsage() const
{
    std::cout
        << "Usage:\n"
        << "  NeoCM compile <input.xaml> [--output <output.naml>]\n"
        << "  NeoCM --help\n";
}

}
