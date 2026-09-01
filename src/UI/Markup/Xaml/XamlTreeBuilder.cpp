#include <NeoWPF/UI/Markup/Xaml/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>
#include <NeoWPF/UI/Markup/Xaml/XamlPropertyBinding.h>

#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Image/Image.h>
#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>
#include <NeoWPF/UI/Layout/Grid/ColumnDefinition.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {
namespace {

std::wstring ToWideString(std::string_view value)
{
    std::wstring result;
    result.reserve(value.size());
    for (const char ch : value) {
        result.push_back(static_cast<wchar_t>(ch));
    }
    return result;
}

void ApplyAttributes(const XamlNode& node, NeoWPF::UI::Properties::DependencyObject& target)
{
    for (const auto& [name, value] : node.Attributes) {
        const auto* property = target.FindDependencyProperty(name);
        if (!property) {
            continue;
        }

        std::any parsedValue;
        if (property->TryParseValue(value, parsedValue)) {
            target.SetValue(*property, parsedValue);
        }
    }
}

void ApplyTextContent(const XamlNode& node, NeoWPF::UI::Properties::DependencyObject& target)
{
    if (node.TextContent.empty()) {
        return;
    }

    const auto* textProperty = target.FindDependencyProperty("Text");
    if (textProperty && textProperty->GetPropertyType() == typeid(std::wstring)) {
        target.SetValue(*textProperty, ToWideString(node.TextContent));
    }
}


std::pair<std::string, std::string> SplitDottedName(const std::string& dottedName)
{
    const auto dot = dottedName.find('.');
    if (dot == std::string::npos) {
        return {dottedName, ""};
    }
    return {dottedName.substr(0, dot), dottedName.substr(dot + 1)};
}

}

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlObjectInternal(const XamlNode& node)
{

    const auto* typeInfo = XamlTypeRegistry::Get().Find(node.Name);
    if (!typeInfo) {
        NeoWPF::Diagnostics::QueueDeveloperWarning(
            L"XAML_TYPE_NOT_FOUND",
            L"Unknown XAML type: " + ToWideString(node.Name),
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
        return nullptr;
    }


    auto obj = typeInfo->Factory();
    if (!obj) {
        return nullptr;
    }


    ApplyAttributes(node, *obj);


    ApplyTextContent(node, *obj);


    for (const auto& childNode : node.Children) {
        if (childNode.Name.empty()) {
            continue;
        }


        const auto [ownerName, propName] = SplitDottedName(childNode.Name);
        if (!propName.empty()) {

            const auto* prop = obj->FindDependencyProperty(propName);
            if (!prop) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_PROPERTY_NOT_FOUND",
                    L"Unknown property: " + ToWideString(propName),
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
                continue;
            }

            const auto* binding = prop->GetXamlBinding();
            if (!binding) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_NO_BINDING",
                    L"Property not registered for XAML: " + ToWideString(propName),
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
                continue;
            }


            if (binding->Kind == XamlPropertyKind::Collection) {

                for (const auto& grandChild : childNode.Children) {
                    auto child = BuildXamlObjectInternal(grandChild);
                    if (child) {
                        binding->Assign(*obj, child);
                    }
                }
            } else if (binding->Kind == XamlPropertyKind::Element) {

                if (childNode.Children.size() == 1) {
                    auto child = BuildXamlObjectInternal(childNode.Children[0]);
                    if (child) {
                        binding->Assign(*obj, child);
                    }
                } else if (childNode.Children.empty()) {
                    NeoWPF::Diagnostics::QueueDeveloperWarning(
                        L"XAML_ELEMENT_EMPTY",
                        L"Element property must have exactly one child: " + ToWideString(propName),
                        NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
                }
            }
        } else {
            auto child = BuildXamlObjectInternal(childNode);
            if (!child) {
                continue;
            }

            const auto* typeInfo2 = XamlTypeRegistry::Get().Find(node.Name);
            if (typeInfo2 && !typeInfo2->ContentPropertyName.empty()) {
                const auto* contentProp = obj->FindDependencyProperty(typeInfo2->ContentPropertyName);
                if (contentProp) {
                    const auto* binding = contentProp->GetXamlBinding();
                    if (binding && binding->Kind == XamlPropertyKind::Collection) {
                        binding->Assign(*obj, child);
                        continue;
                    }
                }
            }

            if (auto visualOwner = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(obj)) {
                if (auto visualChild = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(child)) {
                    visualOwner->AddChild(visualChild);
                }
            }
        }
    }

    return obj;
}

std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const XamlNode& root)
{
    auto obj = BuildXamlObjectInternal(root);
    if (!obj) {
        return nullptr;
    }


    auto visual = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(obj);
    if (!visual) {

        return nullptr;
    }

    return visual;
}

std::shared_ptr<NeoWPF::UI::Visual> BuildXamlTree(const std::vector<XamlNode>& roots)
{
    if (roots.empty()) {
        return nullptr;
    }

    if (roots.size() == 1) {
        return BuildXamlTree(roots.front());
    }

    auto rootContainer = std::make_shared<NeoWPF::UI::StackPanel>();
    for (const auto& root : roots) {
        auto built = BuildXamlTree(root);
        if (built) {
            rootContainer->AddChild(built);
        }
    }
    return rootContainer;
}

}
