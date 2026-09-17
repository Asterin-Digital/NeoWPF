#include <NeoCM/Core/Compiler/NamlCompiler.h>
#include <NeoCM/Core/Compiler/XamlValidation.h>

#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlParser.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <algorithm>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace NeoCM::Core::Compiler {
namespace {

using XamlNode = NeoWPF::UI::Markup::Xaml::XamlNode;

class ArtifactBuilder {
public:
    explicit ArtifactBuilder(NeoNaml::NamlArtifact& artifact)
        : m_artifact(artifact)
    {
    }

    bool AddRoot(const NeoWPF::UI::Markup::Xaml::XamlNode& node)
    {
        m_artifact.rootObjectId = AddNode(node);
        return !m_failed;
    }

private:
    using XamlNode = NeoWPF::UI::Markup::Xaml::XamlNode;

    bool AddMarkupExtension(std::uint32_t objectId,
                            std::uint32_t propertyId,
                            std::string_view value,
                            std::size_t line,
                            std::size_t column)
    {
        std::string error;
        const auto extension = NeoWPF::UI::Markup::Xaml::ParseXamlMarkupExtension(
            value,
            {line, column},
            error);
        if (!extension) {
            m_failed = true;
            m_error = error.empty() ? "Invalid markup extension" : error;
            return false;
        }

        NeoWPF::UI::Markup::Xaml::XamlBindingInfo binding;
        if (NeoWPF::UI::Markup::Xaml::TryGetXamlBinding(*extension, binding)) {
            m_artifact.bindings.push_back({objectId, propertyId, std::move(binding.Path), std::move(binding.Mode)});
            return true;
        }

        std::string resourceKey;
        if (NeoWPF::UI::Markup::Xaml::TryGetXamlStaticResource(*extension, resourceKey)) {
            m_artifact.properties.push_back({
                objectId,
                propertyId,
                {NeoNaml::NamlValueKind::MarkupExtension, std::string(value)}});
            return true;
        }

        std::string localeKey;
        if (NeoWPF::UI::Markup::Xaml::TryGetXamlLocaleKey(*extension, localeKey)) {
            m_artifact.properties.push_back({
                objectId,
                propertyId,
                {NeoNaml::NamlValueKind::MarkupExtension, std::string(value)}});
            return true;
        }

        m_failed = true;
        m_error = "Unsupported markup extension at " + std::to_string(line) + ":" + std::to_string(column);
        return false;
    }

    std::uint32_t StringId(const std::string& value)
    {
        const auto existing = m_stringIds.find(value);
        if (existing != m_stringIds.end()) {
            return existing->second;
        }
        const auto id = static_cast<std::uint32_t>(m_artifact.strings.size());
        m_artifact.strings.push_back(value);
        m_stringIds.emplace(value, id);
        return id;
    }

    std::string BoundContentProperty(const XamlNode& node) const
    {
        const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(node.Name);
        if (!typeInfo || typeInfo->ContentPropertyName.empty() || !typeInfo->Factory) {
            return {};
        }

        const auto object = typeInfo->Factory();
        const auto* property = object
            ? object->FindDependencyProperty(typeInfo->ContentPropertyName)
            : nullptr;
        return property && property->GetXamlBinding()
            ? typeInfo->ContentPropertyName
            : std::string();
    }

    std::uint32_t AddNode(const XamlNode& node)
    {
        if (node.Name.empty()) {
            m_failed = true;
            m_error = "XAML contains an unnamed element at " + std::to_string(node.Line) + ":" + std::to_string(node.Column);
            return 0;
        }

        const auto objectId = static_cast<std::uint32_t>(m_artifact.objects.size());
        m_artifact.objects.push_back({objectId, StringId(node.Name), node.XamlName, node.XamlKey});

        std::vector<std::pair<std::string, std::string>> attributes(node.Attributes.begin(), node.Attributes.end());
        std::sort(attributes.begin(), attributes.end());
        for (const auto& [name, value] : attributes) {
            const auto propertyId = StringId(name);
            if (!value.empty() && value.front() == '{') {
                AddMarkupExtension(objectId, propertyId, value, node.Line, node.Column);
                continue;
            }
            m_artifact.properties.push_back({objectId, propertyId, {NeoNaml::NamlValueKind::String, value}});
        }

        if (!node.TextContent.empty()) {
            m_artifact.properties.push_back({objectId, StringId("Text"), {NeoNaml::NamlValueKind::String, node.TextContent}});
        }

        for (const auto& child : node.Children) {
            if (child.Name.empty()) {
                continue;
            }
            const auto dot = child.Name.find('.');
            const auto propertyName = dot == std::string::npos ? std::string() : child.Name.substr(dot + 1);
            if (!propertyName.empty()) {
                if (propertyName == "Resources") {
                    if (child.Children.size() == 1 && child.Children.front().Name == "ResourceDictionary") {
                        for (const auto& resource : child.Children.front().Children) {
                            const auto childId = AddNode(resource);
                            if (m_failed) return objectId;
                            m_artifact.children.push_back({objectId, childId, StringId("Resources")});
                        }
                    }
                    continue;
                }
                for (const auto& propertyChild : child.Children) {
                    const auto childId = AddNode(propertyChild);
                    if (m_failed) return objectId;
                    m_artifact.children.push_back({objectId, childId, StringId(propertyName)});
                }
                continue;
            }

            const auto childId = AddNode(child);
            if (m_failed) return objectId;
            const auto contentProperty = BoundContentProperty(node);
            m_artifact.children.push_back({
                objectId,
                childId,
                StringId(contentProperty.empty()
                    ? std::string(NeoNaml::ImplicitChildrenPropertyName)
                    : contentProperty)});
        }
        return objectId;
    }

public:
    std::string Error() const { return m_error; }

private:
    NeoNaml::NamlArtifact& m_artifact;
    std::unordered_map<std::string, std::uint32_t> m_stringIds;
    bool m_failed = false;
    std::string m_error;
};

}

NamlCompileResult CompileXaml(const std::string& source, const std::string& sourcePath)
{
    NamlCompileResult result;
    result.artifact.sourcePath = sourcePath;
    result.artifact.resourceKey = std::filesystem::path(sourcePath).stem().string();

    NeoWPF::UI::Properties::RegisterCoreControlProperties();
    NeoWPF::UI::Markup::Xaml::RegisterXamlMarkupExtensions();

    std::vector<XamlNode> roots;
    try {
        roots = NeoWPF::UI::Markup::Xaml::ParseXaml(source);
    } catch (const std::exception& exception) {
        result.error = "XAML parse failed for " + sourcePath + ": " + exception.what();
        return result;
    }
    if (roots.empty()) {
        result.error = "XAML did not contain a root element: " + sourcePath;
        return result;
    }

    if (!ValidateXaml(roots, result.error)) {
        result.error = "XAML validation failed for " + sourcePath + ": " + result.error;
        return result;
    }

    ArtifactBuilder builder(result.artifact);
    if (!builder.AddRoot(roots.front())) {
        result.error = builder.Error();
        return result;
    }

    result.success = true;
    return result;
}

}
