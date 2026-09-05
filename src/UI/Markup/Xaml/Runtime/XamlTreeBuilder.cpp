#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTreeBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlResourceBuilder.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Data/BindingOperations.h>

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
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlObjectInternal(
    const XamlNode& node,
    XamlNamescope* namescope,
    XamlBuildContext& context);

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

void ApplyAttributes(const XamlNode& node,
                     NeoWPF::UI::Properties::DependencyObject& target,
                     const XamlBuildContext& context)
{
    for (const auto& [name, value] : node.Attributes) {
        const auto* property = target.FindDependencyProperty(name);
        if (!property) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_PROPERTY_NOT_FOUND",
                L"Unknown attribute property '" + ToWideString(name) + L"' on XAML object at " +
                    ToWideString(std::to_string(node.Line)) + L":" + ToWideString(std::to_string(node.Column)) +
                    L" (value='" + ToWideString(value) + L"')",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            continue;
        }

        const auto trimmedValue = [&value] {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) {
                return std::string();
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, last - first + 1);
        }();

        if (!trimmedValue.empty() && trimmedValue.front() == '{') {
            std::string error;
            XamlMarkupExtensionContext extensionContext{node.Line, node.Column,
                CreateXamlResourceResolver(target, context)};
            const auto extension = ParseXamlMarkupExtension(trimmedValue, extensionContext, error);
            if (!extension || !extension->Apply(target, *property, error)) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_MARKUP_EXTENSION",
                    ToWideString(error.empty() ? "markup extension could not be applied" : error) +
                        L" on property '" + ToWideString(name) + L"'",
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
                continue;
            }
            continue;
        }

        std::any parsedValue;
        if (property->TryParseValue(value, parsedValue)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                "[XamlTreeBuilder] Attribute applied object=%p property='%s' parsedType='%s'",
                &target, name.c_str(), parsedValue.type().name());
            target.SetValue(*property, parsedValue);
        } else {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Info,
                "[XamlTreeBuilder] Attribute conversion failed object=%p property='%s' value='%s'",
                &target, name.c_str(), value.c_str());
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

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlObjectInternal(
    const XamlNode& node,
    XamlNamescope* namescope,
    XamlBuildContext& context)
{

    const auto* typeInfo = XamlTypeRegistry::Get().Find(node.Name);
    if (!typeInfo) {
        NeoWPF::Diagnostics::QueueDeveloperWarning(
            L"XAML_TYPE_NOT_FOUND",
            L"Unknown XAML type: " + ToWideString(node.Name),
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
        return nullptr;
    }


    auto obj = typeInfo->Factory();
    if (!obj) {
        return nullptr;
    }

    if (namescope && !node.XamlName.empty() && !namescope->Register(node.XamlName, obj)) {
        NeoWPF::Diagnostics::QueueDeveloperWarning(
            L"XAML_NAME_SCOPE",
            L"Duplicate or invalid XAML name: " + ToWideString(node.XamlName),
            NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
        "[XamlTreeBuilder] Created tag='%s' object=%p contentProperty='%s' attributes=%zu children=%zu",
        node.Name.c_str(), obj.get(), typeInfo->ContentPropertyName.c_str(), node.Attributes.size(), node.Children.size());


    if (auto* element = dynamic_cast<NeoWPF::UI::UIElement*>(obj.get())) {
        context.scopes.push_back(element);
    }

    for (const auto& childNode : node.Children) {
        const auto [ownerName, propName] = SplitDottedName(childNode.Name);
        if (!propName.empty() && IsXamlResourcesProperty(propName)) {
            BuildXamlResourceDictionary(childNode, *obj, namescope, context, BuildXamlObjectInternal);
        }
    }

    ApplyAttributes(node, *obj, context);


    ApplyTextContent(node, *obj);


    for (const auto& childNode : node.Children) {
        if (childNode.Name.empty()) {
            continue;
        }


        const auto [ownerName, propName] = SplitDottedName(childNode.Name);
        if (!propName.empty()) {

            if (IsXamlResourcesProperty(propName)) {
                continue;
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                "[XamlTreeBuilder] Property element parent='%s' property='%s' children=%zu",
                node.Name.c_str(), propName.c_str(), childNode.Children.size());

            const auto* prop = obj->FindDependencyProperty(propName);
            if (!prop) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_PROPERTY_NOT_FOUND",
                    L"Unknown property: " + ToWideString(propName),
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
                continue;
            }

            const auto* binding = prop->GetXamlBinding();
            if (!binding) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_NO_BINDING",
                    L"Property not registered for XAML: " + ToWideString(propName),
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
                continue;
            }

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                "[XamlTreeBuilder] Binding found parent='%s' property='%s' kind=%d",
                node.Name.c_str(), propName.c_str(), static_cast<int>(binding->Kind));


            if (binding->Kind == XamlPropertyKind::Collection) {

                for (const auto& grandChild : childNode.Children) {
                    auto child = BuildXamlObjectInternal(grandChild, namescope, context);
                    if (child) {
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                            "[XamlTreeBuilder] Assign collection parent='%s' property='%s' child=%p",
                            node.Name.c_str(), propName.c_str(), child.get());
                        binding->Assign(*obj, child);
                    }
                }
            } else if (binding->Kind == XamlPropertyKind::Element) {

                if (childNode.Children.size() == 1) {
                    auto child = BuildXamlObjectInternal(childNode.Children[0], namescope, context);
                    if (child) {
                        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                            "[XamlTreeBuilder] Assign element parent='%s' property='%s' child=%p",
                            node.Name.c_str(), propName.c_str(), child.get());
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
            auto child = BuildXamlObjectInternal(childNode, namescope, context);
            if (!child) {
                continue;
            }

            const auto* typeInfo2 = XamlTypeRegistry::Get().Find(node.Name);
            if (typeInfo2 && !typeInfo2->ContentPropertyName.empty()) {
                const auto* contentProp = obj->FindDependencyProperty(typeInfo2->ContentPropertyName);
                if (contentProp) {
                    const auto* binding = contentProp->GetXamlBinding();
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                        "[XamlTreeBuilder] Default content parent='%s' property='%s' binding=%s kind=%d child=%p",
                        node.Name.c_str(), typeInfo2->ContentPropertyName.c_str(), binding ? "yes" : "no",
                        binding ? static_cast<int>(binding->Kind) : -1, child.get());
                    if (binding && (binding->Kind == XamlPropertyKind::Collection || binding->Kind == XamlPropertyKind::Element)) {
                        binding->Assign(*obj, child);
                        continue;
                    }
                } else {
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Info,
                        "[XamlTreeBuilder] Default content property missing parent='%s' property='%s' object=%p",
                        node.Name.c_str(), typeInfo2->ContentPropertyName.c_str(), obj.get());
                }
            }

            if (auto visualOwner = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(obj)) {
                if (auto visualChild = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(child)) {
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::XamlParser, NeoWPF::Diagnostics::Level::Debug,
                        "[XamlTreeBuilder] Add visual child parent='%s' parentObject=%p child=%p",
                        node.Name.c_str(), obj.get(), child.get());
                    visualOwner->AddChild(visualChild);
                }
            }
        }
    }

    if (dynamic_cast<NeoWPF::UI::UIElement*>(obj.get()) && !context.scopes.empty()) {
        context.scopes.pop_back();
    }
    return obj;
}

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlTreeFromRoot(
    const XamlNode& root,
    XamlNamescope* namescope)
{
    XamlBuildContext context;
    auto obj = BuildXamlObjectInternal(root, namescope, context);
    if (!obj) {
        return nullptr;
    }


    return obj;
}

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlTree(
    const std::vector<XamlNode>& roots,
    XamlNamescope* namescope)
{
    NEO_WPF_ACTION("Xaml.BuildTree");

    if (roots.empty()) {
        return nullptr;
    }

    if (roots.size() == 1) {
        return BuildXamlTreeFromRoot(roots.front(), namescope);
    }

    auto rootContainer = std::make_shared<NeoWPF::UI::StackPanel>();
    XamlBuildContext context;
    for (const auto& root : roots) {
        auto built = BuildXamlObjectInternal(root, namescope, context);
        auto visual = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(built);
        if (visual) {
            rootContainer->AddChild(visual);
        }
    }
    return rootContainer;
}

std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> BuildXamlTree(
    const XamlNode& root,
    XamlNamescope* namescope)
{
    NEO_WPF_ACTION("Xaml.BuildTree");
    return BuildXamlTreeFromRoot(root, namescope);
}

}
