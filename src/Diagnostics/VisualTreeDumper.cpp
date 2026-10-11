#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <typeinfo>
#include <sstream>
#include <iomanip>
#include <vector>

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

        LogInfo("%s", elementInfo.c_str());
    } else {

        std::string connector = isLast ? "`-- " : "|-- ";
        LogInfo("%s%s%s", prefix.c_str(), connector.c_str(), elementInfo.c_str());
    }

    const auto& children = element->GetVisualChildren();
    if (!children.empty()) {

        std::string newPrefix = prefix;
        if (currentDepth > 0) {
            newPrefix += (isLast ? "    " : "|   ");
        } else {

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

static std::string FormatArgb(std::uint32_t argb)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << argb;
    return oss.str();
}

static std::string FormatBrush(const std::shared_ptr<NeoWPF::UI::Brush>& brush)
{
    if (!brush) return "null";
    if (const auto solidBrush = std::dynamic_pointer_cast<NeoWPF::UI::SolidColorBrush>(brush)) {
        return FormatArgb(solidBrush->GetArgb());
    }
    return typeid(*brush).name();
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
    bool wroteAny = false;

    if (auto uiElement = dynamic_cast<NeoWPF::UI::UIElement*>(element)) {
        oss << "State{";
        oss << "Visible=" << (uiElement->IsVisible() ? "Yes" : "No");
        oss << ",Enabled=" << (uiElement->IsEnabled() ? "Yes" : "No");
        oss << ",HitTestVisible=" << (uiElement->IsHitTestVisible() ? "Yes" : "No");
        oss << ",Interactable=" << (uiElement->IsInteractable() ? "Yes" : "No");
        oss << ",Opacity=" << uiElement->GetOpacity();
        oss << ",Template=" << (uiElement->GetTemplate() ? "Yes" : "No");
        oss << "}";
        wroteAny = true;
    }

    if (auto frameworkElement = dynamic_cast<NeoWPF::UI::FrameworkElement*>(element)) {
        if (wroteAny) {
            oss << ", ";
        }
        oss << "LayoutState{";
        oss << "Desired=" << frameworkElement->GetDesiredWidth() << "x" << frameworkElement->GetDesiredHeight();
        oss << ",Final=" << frameworkElement->GetFinalWidth() << "x" << frameworkElement->GetFinalHeight();
        oss << ",Offset=" << frameworkElement->GetLayoutOffsetX() << "," << frameworkElement->GetLayoutOffsetY();
        oss << ",Width=" << frameworkElement->GetWidth() << ",Height=" << frameworkElement->GetHeight();
        auto margin = frameworkElement->GetMargin();
        oss << ",Margin=" << margin.Left << "," << margin.Top << "," << margin.Right << "," << margin.Bottom;
        oss << "}";
        wroteAny = true;
    }

    if (auto control = dynamic_cast<NeoWPF::UI::Control*>(element)) {
        if (wroteAny) {
            oss << ", ";
        }
        oss << "VisualProps{";
        const auto padding = control->GetPadding();
        oss << "Padding=" << padding.Left << "," << padding.Top << "," << padding.Right << "," << padding.Bottom;
        oss << ",Bg=" << FormatBrush(control->GetBackgroundBrush());
        oss << ",Border=" << FormatBrush(control->GetBorderBrush());
        oss << ",BorderThickness=" << control->GetBorderThickness();
        oss << "}";
        wroteAny = true;
    }

    if (auto templateVisual = dynamic_cast<NeoWPF::UI::Templates::TemplateVisual*>(element)) {
        if (wroteAny) {
            oss << ", ";
        }
        oss << "TemplateVisual{";
        oss << "Rect=" << templateVisual->GetX() << "," << templateVisual->GetY() << "," << templateVisual->GetWidth() << "x" << templateVisual->GetHeight();
        oss << ",Opacity=" << templateVisual->GetOpacity();
        oss << ",Rotation=" << templateVisual->GetRotation();
        oss << "}";
        wroteAny = true;
    }

    if (!wroteAny) {
        return "";
    }

    return oss.str();
}

std::string VisualTreeDumper::FormatStylesInfo(NeoWPF::UI::Visual* element)
{
    std::ostringstream oss;
    bool wroteAny = false;

    if (auto uiElement = dynamic_cast<NeoWPF::UI::UIElement*>(element)) {
        if (uiElement->GetTemplate()) {
            oss << "Template=Yes";
            wroteAny = true;
        }

        if (uiElement->GetTemplateVisualStateManager()) {
            if (wroteAny) {
                oss << ", ";
            }
            oss << "VisualStateManager=Yes";
            wroteAny = true;
        }

        auto templateChildren = uiElement->GetTemplateChildNames();
        if (!templateChildren.empty()) {
            if (wroteAny) {
                oss << ", ";
            }
            oss << "TemplateChildren={";
            for (size_t i = 0; i < templateChildren.size(); ++i) {
                if (i > 0) {
                    oss << ",";
                }
                oss << templateChildren[i];
            }
            oss << "}";
            wroteAny = true;
        }
    }

    if (!wroteAny) {
        return "";
    }

    return std::string("Styles{") + oss.str() + "}";
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
