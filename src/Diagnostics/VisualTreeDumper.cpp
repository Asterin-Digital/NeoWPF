#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <typeinfo>
#include <sstream>

namespace NeoWPF::Diagnostics {

void VisualTreeDumper::DumpTree(NeoWPF::UI::Visual* root, const VisualTreeDumpOptions& options)
{
    if (!root) {
        LogWarn("VisualTreeDumper::DumpTree - root element is null");
        return;
    }

    LogInfo("=== Visual Tree Dump ===");
    std::string prefix;
    DumpTreeRecursive(root, options, 0, true, prefix);
    LogInfo("=== End Visual Tree ===");
}

void VisualTreeDumper::DumpTreeRecursive(
    NeoWPF::UI::Visual* element,
    const VisualTreeDumpOptions& options,
    int currentDepth,
    bool isLast,
    std::string& prefix)
{
    if (!element) return;

    if (options.maxDepth > 0 && currentDepth >= options.maxDepth) {
        return;
    }

    std::string elementInfo = FormatElementInfo(element, options);
    
    if (currentDepth == 0) {
        // Root element - no prefix
        LogInfo("%s", elementInfo.c_str());
    } else {
        // Child element - add tree connector with indentation
        std::string connector = isLast ? "`-- " : "|-- ";
        LogInfo("%s%s%s", prefix.c_str(), connector.c_str(), elementInfo.c_str());
    }

    const auto& children = element->GetVisualChildren();
    if (!children.empty()) {
        // Build prefix for children
        std::string newPrefix = prefix;
        if (currentDepth > 0) {
            newPrefix += (isLast ? "    " : "|   ");
        } else {
            // First level - start indentation
            newPrefix = "";
        }

        for (size_t i = 0; i < children.size(); ++i) {
            bool isLastChild = (i == children.size() - 1);
            DumpTreeRecursive(children[i], options, currentDepth + 1, isLastChild, newPrefix);
        }
    }
}

std::string VisualTreeDumper::FormatElementInfo(
    NeoWPF::UI::Visual* element,
    const VisualTreeDumpOptions& options)
{
    std::ostringstream oss;


    std::string typeName = GetTypeName(element);
    const auto& children = element->GetVisualChildren();
    oss << typeName << " [" << children.size() << " children]";


    if (options.flags & IncludeLayout) {
        std::string layoutInfo = FormatLayoutInfo(element);
        if (!layoutInfo.empty()) {
            oss << " | " << layoutInfo;
        }
    }


    if (options.flags & IncludeProperties) {
        std::string propsInfo = FormatPropertiesInfo(element);
        if (!propsInfo.empty()) {
            oss << " | " << propsInfo;
        }
    }


    if (options.flags & IncludeStyles) {
        std::string stylesInfo = FormatStylesInfo(element);
        if (!stylesInfo.empty()) {
            oss << " | " << stylesInfo;
        }
    }


    if (options.flags & IncludeDiagnostics) {
        std::string diagInfo = FormatDiagnosticsInfo(element);
        if (!diagInfo.empty()) {
            oss << " | " << diagInfo;
        }
    }

    return oss.str();
}

std::string VisualTreeDumper::GetTypeName(NeoWPF::UI::Visual* element)
{
    if (!element) return "null";


    const std::type_info& typeInfo = typeid(*element);
    std::string name = typeInfo.name();



    size_t lastColonPos = name.rfind(':');
    if (lastColonPos != std::string::npos && lastColonPos + 1 < name.length()) {
        return name.substr(lastColonPos + 1);
    }


    return name;
}

std::string VisualTreeDumper::FormatLayoutInfo(NeoWPF::UI::Visual* element)
{

    auto* frameworkElem = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element);
    if (!frameworkElem) {
        return "";
    }

    std::ostringstream oss;
    oss << "Layout{";
    oss << "W:" << frameworkElem->GetFinalWidth()
        << ",H:" << frameworkElem->GetFinalHeight()
        << ",X:" << frameworkElem->GetLayoutOffsetX()
        << ",Y:" << frameworkElem->GetLayoutOffsetY();


    if (!frameworkElem->IsVisible()) {
        oss << ",Hidden";
    }
    if (!frameworkElem->IsEnabled()) {
        oss << ",Disabled";
    }

    oss << "}";
    return oss.str();
}

std::string VisualTreeDumper::FormatPropertiesInfo(NeoWPF::UI::Visual* element)
{




    std::ostringstream oss;
    oss << "Props{}";
    return oss.str();
}

std::string VisualTreeDumper::FormatStylesInfo(NeoWPF::UI::Visual* element)
{



    std::ostringstream oss;
    oss << "Styles{}";
    return oss.str();
}

std::string VisualTreeDumper::FormatDiagnosticsInfo(NeoWPF::UI::Visual* element)
{
    std::ostringstream oss;
    oss << "Diag{";
    if (element->IsTreeDirty()) {
        oss << "Dirty";
    } else {
        oss << "Clean";
    }
    oss << "}";
    return oss.str();
}

}
