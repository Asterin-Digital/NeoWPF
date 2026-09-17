#include <NeoCM/Core/Compiler/ResolvedXaml.h>
#include <NeoCM/Core/Compiler/XamlValidation.h>

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <algorithm>
#include <utility>

namespace NeoCM::Core::Compiler {
namespace {

using XamlNode = NeoWPF::UI::Markup::Xaml::XamlNode;

const NeoWPF::UI::Properties::DependencyProperty* FindProperty(
    NeoWPF::UI::Properties::DependencyObject& object,
    const std::string& name)
{
    return object.FindDependencyProperty(name);
}

std::string Location(const XamlNode& node)
{
    return " at " + std::to_string(node.Line) + ":" + std::to_string(node.Column);
}

bool ResolveProperty(const XamlNode& node,
                     const std::string& name,
                     const std::string& value,
                     ResolvedXamlProperty& result,
                     std::string& error)
{
    const auto dot = name.find('.');
    const auto ownerTag = dot == std::string::npos ? std::string() : name.substr(0, dot);
    const auto propertyName = dot == std::string::npos ? name : name.substr(dot + 1);
    const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
    const auto object = typeInfo && typeInfo->Factory ? typeInfo->Factory() : nullptr;
    const auto* property = object ? FindProperty(*object, name) : nullptr;
    if (!property) {
        error = "XAML property '" + name + "' has no dependency-property metadata" + Location(node);
        return false;
    }

    const auto* ownerInfo = ownerTag.empty()
        ? NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().FindByType(property->GetOwnerType())
        : NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(ownerTag);
    if (!ownerInfo || ownerInfo->CppTypeName.empty()) {
        error = "XAML property '" + name + "' has no C++ owner metadata" + Location(node);
        return false;
    }
    result.propertyExpression = ownerInfo->CppTypeName + "::" + propertyName + "Property()";
    result.name = name;
    result.value = value;
    result.isMarkupExtension = !value.empty() && value.front() == '{';
    if (result.isMarkupExtension) {
        std::string extensionError;
        const auto extension = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
            value,
            {node.Line, node.Column},
            extensionError);
        NeoWPF::UI::Markup::Xaml::XamlBindingInfo binding;
        if (!extension) {
            error = "XAML property '" + name + "' has an unsupported markup extension" + Location(node);
            return false;
        }

        if (NeoWPF::UI::Markup::Xaml::TryGetXamlBinding(*extension, binding)) {
            result.bindingPath = std::move(binding.Path);
            result.bindingMode = std::move(binding.Mode);
            return true;
        }

        if (NeoWPF::UI::Markup::Xaml::TryGetXamlStaticResource(
                *extension, result.staticResourceKey)) {
            return true;
        }

        if (NeoWPF::UI::Markup::Xaml::TryGetXamlLocaleKey(*extension, result.localeKey)) {
            return true;
        }

        error = "XAML property '" + name + "' has an unsupported markup extension" + Location(node);
        return false;
    }

    if (!property->TryConvertToCode(value, result.valueExpression)) {
        error = "XAML property '" + name + "' has no supported C++ code representation" + Location(node);
        return false;
    }
    return true;
}

bool ResolveElement(const XamlNode& node,
                    ResolvedXamlElement& result,
                    std::string& error)
{
    const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
    if (!typeInfo || typeInfo->CppTypeName.empty() || typeInfo->HeaderPath.empty()) {
        error = "XAML type '" + node.Name + "' has no complete code-generation metadata" + Location(node);
        return false;
    }

    result.tagName = node.Name;
    result.cppTypeName = typeInfo->CppTypeName;
    result.headerPath = typeInfo->HeaderPath;
    result.xamlName = node.XamlName;
    result.textContent = node.TextContent;
    result.line = node.Line;
    result.column = node.Column;

    std::vector<std::pair<std::string, std::string>> attributes(node.Attributes.begin(), node.Attributes.end());
    std::sort(attributes.begin(), attributes.end());
    for (const auto& [name, value] : attributes) {
        ResolvedXamlProperty property;
        if (!ResolveProperty(node, name, value, property, error)) {
            return false;
        }
        result.properties.push_back(std::move(property));
    }

    for (const auto& child : node.Children) {
        if (child.Name.empty()) {
            continue;
        }
        const auto dot = child.Name.find('.');
        const auto* nodeType = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
        const auto nodeObject = nodeType && nodeType->Factory ? nodeType->Factory() : nullptr;
        if (dot == std::string::npos && nodeType && !nodeType->ContentPropertyName.empty() && nodeObject) {
            const auto* contentProperty = FindProperty(*nodeObject, nodeType->ContentPropertyName);
            const auto* ownerInfo = contentProperty
                ? NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().FindByType(contentProperty->GetOwnerType())
                : nullptr;
            const auto* binding = contentProperty ? contentProperty->GetXamlBinding() : nullptr;
            if (contentProperty && ownerInfo && !ownerInfo->CppTypeName.empty() && binding &&
                (binding->Kind == NeoWPF::UI::Markup::Xaml::XamlPropertyKind::Element ||
                 binding->Kind == NeoWPF::UI::Markup::Xaml::XamlPropertyKind::Collection)) {
                ResolvedXamlElement resolvedChild;
                if (!ResolveElement(child, resolvedChild, error)) {
                    return false;
                }
                result.children.push_back({
                    nodeType->ContentPropertyName,
                    ownerInfo->CppTypeName + "::" + nodeType->ContentPropertyName + "Property()",
                    binding->CodeGenerationAdder,
                    std::move(resolvedChild)});
                continue;
            }
        }

        const auto propertyName = dot == std::string::npos ? std::string() : child.Name.substr(dot + 1);
        if (!propertyName.empty()) {
            if (propertyName == "Resources") {
                if (child.Children.size() == 1 && child.Children.front().Name == "ResourceDictionary") {
                    for (const auto& resourceNode : child.Children.front().Children) {
                        ResolvedXamlElement resource;
                        if (!ResolveElement(resourceNode, resource, error)) {
                            return false;
                        }
                        result.resources.push_back({
                            resourceNode.XamlKey,
                            resourceNode.XamlKey.empty() && resourceNode.Name == "Style",
                            std::move(resource)});
                    }
                }
                continue;
            }
            const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
            const auto object = typeInfo && typeInfo->Factory ? typeInfo->Factory() : nullptr;
            const auto* property = object ? FindProperty(*object, propertyName) : nullptr;
            const auto* ownerInfo = property
                ? NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().FindByType(property->GetOwnerType())
                : nullptr;
            if (!property || !ownerInfo || ownerInfo->CppTypeName.empty()) {
                error = "XAML property element '" + child.Name + "' has no C++ property metadata" + Location(child);
                return false;
            }
            const auto propertyExpression = ownerInfo->CppTypeName + "::" + propertyName + "Property()";
            const auto* binding = property->GetXamlBinding();
            for (const auto& propertyChild : child.Children) {
                ResolvedXamlElement resolvedChild;
                if (!ResolveElement(propertyChild, resolvedChild, error)) {
                    return false;
                }
                result.children.push_back({
                    propertyName,
                    propertyExpression,
                    binding ? binding->CodeGenerationAdder : std::string(),
                    std::move(resolvedChild)});
            }
            continue;
        }

        ResolvedXamlElement resolvedChild;
        if (!ResolveElement(child, resolvedChild, error)) {
            return false;
        }
        result.children.push_back({{}, {}, {}, std::move(resolvedChild)});
    }

    return true;
}

}

XamlResolveResult ResolveXaml(
    const std::vector<NeoWPF::UI::Markup::Xaml::XamlNode>& roots,
    const std::string& sourcePath)
{
    XamlResolveResult result;
    result.document.sourcePath = sourcePath;

    NeoWPF::UI::Markup::Xaml::RegisterXamlMarkupExtensions();

    if (!ValidateXaml(roots, result.error)) {
        result.error = "XAML validation failed for " + sourcePath + ": " + result.error;
        return result;
    }

    result.document.xamlClass = roots.front().XamlClass;
    const auto* rootType = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(roots.front().Name);
    if (rootType) {
        result.document.rootKind = rootType->Role;
    }

    if (!ResolveElement(roots.front(), result.document.root, result.error)) {
        result.error = "XAML resolution failed for " + sourcePath + ": " + result.error;
        return result;
    }

    result.success = true;
    return result;
}

}
