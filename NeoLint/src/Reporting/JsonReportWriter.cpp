#include <NeoLint/Reporting/JsonReportWriter.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace neolint
{
namespace
{
const char* SeverityName(RuleSeverity severity)
{
    return severity == RuleSeverity::Error ? "error" :
        severity == RuleSeverity::Warning ? "warning" : "info";
}
}

bool JsonReportWriter::Write(const std::filesystem::path& path,
                             const DiagnosticList& diagnostics, std::string& error)
{
    nlohmann::json report = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics)
    {
        report.push_back({
            {"rule", diagnostic.ruleId},
            {"severity", SeverityName(diagnostic.severity)},
            {"file", diagnostic.location.file},
            {"line", diagnostic.location.line},
            {"column", diagnostic.location.column},
            {"message", diagnostic.message},
        });
    }

    std::ofstream output(path);
    if (!output)
    {
        error = "cannot write report '" + path.string() + "'";
        return false;
    }

    output << report.dump(2) << '\n';
    return output.good();
}
}
