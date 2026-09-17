#include <NeoCM/Core/Compiler/XamlValidation.h>

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <any>
#include <string>
#include <unordered_set>
#include <utility>

namespace NeoCM::Core::Compiler {
namespace {

using XamlNode = NeoWPF::UI::Markup::Xaml::XamlNode;

std::pair<std::string, std::string> SplitPropertyElementName(const std::string& name)
{
    const auto dot = name.find('.');
    if (dot == std::string::npos) {
        return {name, {}};
    }
    return {name.substr(0, dot), name.substr(dot + 1)};
}

std::string Location(const XamlNode& node)
{
    return " at " + std::to_string(node.Line) + ":" + std::to_string(node.Column);
}

bool ValidateNode(const XamlNode& node,
                  std::unordered_set<std::string>& names,
                  const std::vector<std::unordered_set<std::string>>& scopes,
                  std::string& error,
                  bool isRoot)
{
    const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
    if (!typeInfo || !typeInfo->Factory) {
        error = "unknown XAML type '" + node.Name + "'" + Location(node);
        return false;
    }

    if (!node.XamlName.empty() && !names.insert(node.XamlName).second) {
        error = "duplicate XAML name '" + node.XamlName + "'" + Location(node);
        return false;
    }

    if (!isRoot && !node.XamlClass.empty()) {
        error = "x:Class is only valid on the XAML root" + Location(node);
        return false;
    }

    const auto object = typeInfo->Factory();
    if (!object) {
        error = "XAML type '" + node.Name + "' could not be constructed" + Location(node);
        return false;
    }

    std::unordered_set<std::string> localResources;
    std::unordered_set<std::string> localImplicitStyles;
    for (const auto& child : node.Children) {
        const auto propertyElement = SplitPropertyElementName(child.Name);
        if (propertyElement.second != "Resources") {
            continue;
        }
        if (child.Children.size() != 1 || child.Children.front().Name != "ResourceDictionary") {
            error = "resources property must contain exactly one ResourceDictionary" + Location(child);
            return false;
        }
        for (const auto& resource : child.Children.front().Children) {
            if (resource.Name.empty()) continue;
            if (resource.XamlKey.empty()) {
                if (resource.Name != "Style") {
                    error = "resource dictionary entry is missing x:Key" + Location(resource);
                    return false;
                }
                const auto targetType = resource.Attributes.find("TargetType");
                if (targetType == resource.Attributes.end() || targetType->second.empty()) {
                    error = "implicit Style resource requires TargetType" + Location(resource);
                    return false;
                }
                if (!localImplicitStyles.insert(targetType->second).second) {
                    error = "duplicate implicit style target type '" + targetType->second + "'" + Location(resource);
                    return false;
                }
                continue;
            }
            if (!localResources.insert(resource.XamlKey).second) {
                error = "duplicate resource key '" + resource.XamlKey + "'" + Location(resource);
                return false;
            }
        }
    }

    auto activeScopes = scopes;
    if (!localResources.empty()) {
        activeScopes.push_back(localResources);
    }

    for (const auto& [propertyName, value] : node.Attributes) {
        const auto* property = object->FindDependencyProperty(propertyName);
        if (!property) {
            error = "unknown property '" + propertyName + "' on '" + node.Name + "'" + Location(node);
            return false;
        }

        if (!value.empty() && value.front() == '{') {
            std::string extensionError;
            if (!NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
                    value, {node.Line, node.Column}, extensionError)) {
                error = (extensionError.empty() ? "invalid markup extension" : extensionError) + Location(node);
                return false;
            }

            const auto extension = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
                value, {node.Line, node.Column}, extensionError);
            std::string resourceKey;
            if (extension && NeoWPF::UI::Markup::Xaml::TryGetXamlStaticResource(*extension, resourceKey)) {
                const auto found = std::find_if(
                    activeScopes.rbegin(),
                    activeScopes.rend(),
                    [&resourceKey](const auto& scope) { return scope.contains(resourceKey); });
                if (found == activeScopes.rend()) {
                    error = "StaticResource '" + resourceKey + "' was not found in scope" + Location(node);
                    return false;
                }
            }
            continue;
        }

        std::any parsedValue;
        if (!property->TryParseValue(value, parsedValue)) {
            error = "invalid value for property '" + propertyName + "' on '" + node.Name + "'" + Location(node);
            return false;
        }
    }

    if (!node.TextContent.empty()) {
        const auto* textProperty = object->FindDependencyProperty("Text");
        std::any parsedText;
        if (!textProperty || !textProperty->TryParseValue(node.TextContent, parsedText)) {
            error = "text content is not supported by '" + node.Name + "'" + Location(node);
            return false;
        }
    }

    for (const auto& child : node.Children) {
        if (child.Name.empty()) {
            continue;
        }

        const auto propertyElement = SplitPropertyElementName(child.Name);
        const auto& propertyName = propertyElement.second;
        if (!propertyName.empty()) {
            if (propertyName == "Resources") {
                for (const auto& resource : child.Children.front().Children) {
                    if (resource.Name.empty()) continue;
                    if (!ValidateNode(resource, names, activeScopes, error, false)) return false;
                }
                continue;
            }

            const auto* property = object->FindDependencyProperty(propertyName);
            const auto* binding = property ? property->GetXamlBinding() : nullptr;
            if (!binding) {
                error = "property element '" + child.Name + "' is not assignable on '" + node.Name + "'" + Location(child);
                return false;
            }
            if (binding->Kind == NeoWPF::UI::Markup::Xaml::XamlPropertyKind::Element && child.Children.size() != 1) {
                error = "element property '" + propertyName + "' requires exactly one child" + Location(child);
                return false;
            }
            if (binding->Kind == NeoWPF::UI::Markup::Xaml::XamlPropertyKind::Collection && child.Children.empty()) {
                error = "collection property '" + propertyName + "' requires a child" + Location(child);
                return false;
            }
            for (const auto& propertyChild : child.Children) {
                if (!ValidateNode(propertyChild, names, activeScopes, error, false)) return false;
            }
            continue;
        }

        if (!ValidateNode(child, names, activeScopes, error, false)) return false;
    }

    return true;
}

}

bool ValidateXaml(const std::vector<NeoWPF::UI::Markup::Xaml::XamlNode>& roots,
                 std::string& error)
{
    error.clear();
    if (roots.size() != 1) {
        error = "XAML must contain exactly one root element";
        return false;
    }

    std::unordered_set<std::string> names;
    return ValidateNode(roots.front(), names, {}, error, true);
}

}
