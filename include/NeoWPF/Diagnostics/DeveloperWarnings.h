#pragma once

#include <optional>
#include <string>

namespace NeoWPF::Diagnostics {

struct DeveloperWarnings {
    static bool s_debugLogging;

    static constexpr const wchar_t* kDanglingChildReferenceCode = L"NEO-UI101";
    static constexpr const wchar_t* kInvalidViewportBindingCode = L"NEO-UI204";
    static constexpr const wchar_t* kInvalidDependencyPropertyCode = L"NEO-UI208";
    static constexpr const wchar_t* kLayoutOverflowDetectedCode = L"NEO-UI013";
    static constexpr const wchar_t* kInvalidGridIndexCode = L"NEO-UI201";
    static constexpr const wchar_t* kInvalidGridSpanCode = L"NEO-UI202";
    static constexpr const wchar_t* kInvalidGridDefinitionCode = L"NEO-UI203";
    static constexpr const wchar_t* kNullGridChildCode = L"NEO-UI205";
    static constexpr const wchar_t* kGridSelfParentingCode = L"NEO-UI206";
    static constexpr const wchar_t* kGridChildTypeMismatchCode = L"NEO-UI207";
};

enum class DeveloperWarningSeverity {
    Warning,
    Error
};

struct DeveloperWarningMessage {
    std::wstring title;
    std::wstring message;
    DeveloperWarningSeverity severity = DeveloperWarningSeverity::Warning;
};

void QueueDeveloperWarning(const std::wstring& title, const std::wstring& message, DeveloperWarningSeverity severity = DeveloperWarningSeverity::Warning);
void ClearDeveloperWarnings();
void ProcessPendingDeveloperWarnings();
std::optional<DeveloperWarningMessage> GetLastWarning() noexcept;
bool IsDeveloperWarningSuppressionEnabled() noexcept;
void SetDeveloperWarningSuppression(bool enabled) noexcept;

}