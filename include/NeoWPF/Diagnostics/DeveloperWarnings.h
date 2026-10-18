/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

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
        UniformGrid capacity overflow warning code.

        @devwarn NEO-UI209 UniformGrid capacity exceeded.
        Triggered when UniformGrid must add effective rows to preserve its children.
    */
    static constexpr const wchar_t* kUniformGridCapacityOverflowCode = L"NEO-UI209";

    /*
        @summary
        Invalid UniformGrid dimension warning code.

        @devwarn NEO-UI210 Invalid UniformGrid dimension.
        Triggered when Rows or Columns is negative.
    */
    static constexpr const wchar_t* kUniformGridInvalidDimensionCode = L"NEO-UI210";

    /*
        @summary
        Invalid UniformGrid FirstColumn warning code.

        @devwarn NEO-UI211 Invalid UniformGrid FirstColumn.
        Triggered when FirstColumn must be clamped to a valid column index.
    */
    static constexpr const wchar_t* kUniformGridInvalidFirstColumnCode = L"NEO-UI211";

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

    /*
        @summary
        XAML property not found warning code.

        @devwarn NEO-XAML101 XAML property not found.
        Triggered when XAML names an attribute or property element that the object does not expose.
    */
    static constexpr const wchar_t* kXamlPropertyNotFoundCode = L"NEO-XAML101";

    /*
        @summary
        XAML markup extension warning code.

        @devwarn NEO-XAML102 XAML markup extension could not be applied.
        Triggered when a binding or other markup extension cannot be parsed or applied.
    */
    static constexpr const wchar_t* kXamlMarkupExtensionCode = L"NEO-XAML102";

    /*
        @summary
        XAML type not found warning code.

        @devwarn NEO-XAML103 XAML type not found.
        Triggered when an XAML element name is not registered with the runtime type registry.
    */
    static constexpr const wchar_t* kXamlTypeNotFoundCode = L"NEO-XAML103";

    /*
        @summary
        XAML namescope warning code.

        @devwarn NEO-XAML104 Invalid XAML namescope entry.
        Triggered when an x:Name value is duplicated or cannot be registered.
    */
    static constexpr const wchar_t* kXamlNameScopeCode = L"NEO-XAML104";

    /*
        @summary
        XAML property binding warning code.

        @devwarn NEO-XAML105 XAML property has no binding metadata.
        Triggered when a property element has no XAML binding metadata.
    */
    static constexpr const wchar_t* kXamlNoBindingCode = L"NEO-XAML105";

    /*
        @summary
        Empty XAML element property warning code.

        @devwarn NEO-XAML106 Empty XAML element property.
        Triggered when an element property does not contain its required child.
    */
    static constexpr const wchar_t* kXamlElementEmptyCode = L"NEO-XAML106";

    /*
        @summary
        XAML resource key warning code.

        @devwarn NEO-XAML107 Invalid XAML resource key.
        Triggered when a resource key is missing, duplicated, or conflicts with an implicit style.
    */
    static constexpr const wchar_t* kXamlResourceKeyCode = L"NEO-XAML107";

    /*
        @summary
        XAML registration warning code.

        @devwarn NEO-XAML108 Invalid XAML registration.
        Triggered when a registered XAML type is missing required factory or metadata.
    */
    static constexpr const wchar_t* kXamlRegistrationInvalidCode = L"NEO-XAML108";
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