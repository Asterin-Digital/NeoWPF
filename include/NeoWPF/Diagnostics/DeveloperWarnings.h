#pragma once

#include <string>

namespace NeoWPF::Diagnostics {

struct DeveloperWarnings {
    static bool s_debugLogging;

    static constexpr const wchar_t* kDanglingChildReferenceCode = L"NEO-UI101";
    static constexpr const wchar_t* kInvalidViewportBindingCode = L"NEO-UI204";
    static constexpr const wchar_t* kLayoutOverflowDetectedCode = L"NEO-UI013";
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
bool IsDeveloperWarningSuppressionEnabled() noexcept;
void SetDeveloperWarningSuppression(bool enabled) noexcept;

}