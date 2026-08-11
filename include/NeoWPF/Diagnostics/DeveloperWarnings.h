#pragma once

#include <optional>
#include <string>

namespace NeoWPF::Diagnostics {

/*
    @summary
    Developer warning codes used by the NeoWPF diagnostics system.

    @details
    These constants represent warning identifiers that are queued at runtime
    and surfaced in the developer diagnostics UI. Each warning maps to a
    specific runtime condition.
*/
struct DeveloperWarnings {
    static bool s_debugLogging;

    /*
        @summary
        Dangling child reference warning code.

        @devwarn NEO-UI101 Dangling child reference detected.
        Triggered when a visual child is attached without active shared ownership.
    */
    static constexpr const wchar_t* kDanglingChildReferenceCode = L"NEO-UI101";

    /*
        @summary
        Invalid viewport binding warning code.

        @devwarn NEO-UI204 Invalid viewport binding.
        Reserved for invalid viewport binding or viewport-related property usage.
    */
    static constexpr const wchar_t* kInvalidViewportBindingCode = L"NEO-UI204";

    /*
        @summary
        Invalid dependency property warning code.

        @devwarn NEO-UI208 Invalid dependency property used.
        Used when an invalid or unregistered dependency property is referenced.
    */
    static constexpr const wchar_t* kInvalidDependencyPropertyCode = L"NEO-UI208";

    /*
        @summary
        Layout overflow warning code.

        @devwarn NEO-UI013 Layout overflow detected.
        Used when layout passes produce an overflow or measurement issues.
    */
    static constexpr const wchar_t* kLayoutOverflowDetectedCode = L"NEO-UI013";

    /*
        @summary
        Invalid grid index warning code.

        @devwarn NEO-UI201 Invalid grid index.
        Triggered by a grid child with invalid row/column indices.
    */
    static constexpr const wchar_t* kInvalidGridIndexCode = L"NEO-UI201";

    /*
        @summary
        Invalid grid span warning code.

        @devwarn NEO-UI202 Invalid grid span.
        Triggered by a grid child with an invalid row/column span.
    */
    static constexpr const wchar_t* kInvalidGridSpanCode = L"NEO-UI202";

    /*
        @summary
        Invalid grid definition warning code.

        @devwarn NEO-UI203 Invalid grid definition.
        Triggered by malformed or invalid grid row/column definitions.
    */
    static constexpr const wchar_t* kInvalidGridDefinitionCode = L"NEO-UI203";

    /*
        @summary
        Null grid child warning code.

        @devwarn NEO-UI205 Null grid child.
        Queued when a null child is added to a grid.
    */
    static constexpr const wchar_t* kNullGridChildCode = L"NEO-UI205";

    /*
        @summary
        Grid self-parenting warning code.

        @devwarn NEO-UI206 Grid self-parenting detected.
        Triggered when a grid is added as a child of itself.
    */
    static constexpr const wchar_t* kGridSelfParentingCode = L"NEO-UI206";

    /*
        @summary
        Grid child type mismatch warning code.

        @devwarn NEO-UI207 Grid child type mismatch.
        Triggered when a grid child cannot be measured because it is not a FrameworkElement.
    */
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