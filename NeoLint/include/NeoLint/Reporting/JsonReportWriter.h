#pragma once

#include <NeoLint/Core/RuleDiagnostic.h>

#include <filesystem>
#include <string>

namespace neolint
{
class JsonReportWriter
{
public:
    static bool Write(const std::filesystem::path& path,
                      const DiagnosticList& diagnostics,
                      std::string& error);
};
}
