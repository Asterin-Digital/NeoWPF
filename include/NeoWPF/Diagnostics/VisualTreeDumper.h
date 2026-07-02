#pragma once

#include <cstdint>
#include <string>

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Diagnostics {

enum VisualTreeDumpFlags : uint32_t {
    TreeOnly           = 0x00,            // tree structure only
    IncludeLayout      = 0x01,            // layout info (size, offset, alignment)
    IncludeProperties  = 0x02,            // element properties (colors, fonts, etc.)
    IncludeStyles      = 0x04,            // styling information
    IncludeDiagnostics = 0x08,            // diagnostic info (dirty flags, template state)
};

struct VisualTreeDumpOptions {
    uint32_t flags = TreeOnly;             // dmp flags (combination of VisualTreeDumpFlags)
    int maxDepth = 0;                      // max recursion depth (0 = unlimited)

    VisualTreeDumpOptions() = default;
    VisualTreeDumpOptions(uint32_t inFlags, int inMaxDepth = 0)
        : flags(inFlags), maxDepth(inMaxDepth) {}
};

class VisualTreeDumper {
public:
    static void DumpTree(NeoWPF::UI::Visual* root, const VisualTreeDumpOptions& options = VisualTreeDumpOptions{});

private:
    VisualTreeDumper() = delete;

    static void DumpTreeRecursive(
        NeoWPF::UI::Visual* element,
        const VisualTreeDumpOptions& options,
        int currentDepth,
        bool isLast,
        std::string& prefix);

    static std::string FormatElementInfo(
        NeoWPF::UI::Visual* element,
        const VisualTreeDumpOptions& options);

    static std::string GetTypeName(NeoWPF::UI::Visual* element);

    static std::string FormatLayoutInfo(NeoWPF::UI::Visual* element);

    static std::string FormatPropertiesInfo(NeoWPF::UI::Visual* element);

    static std::string FormatStylesInfo(NeoWPF::UI::Visual* element);

    static std::string FormatDiagnosticsInfo(NeoWPF::UI::Visual* element);
};

}
