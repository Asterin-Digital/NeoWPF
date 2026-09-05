#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlResourceBuilder.h>

#include <NeoWPF/Diagnostics/DeveloperWarnings.h>

#include <array>
#include <string>
#include <unordered_set>

namespace NeoWPF::UI::Markup::Xaml {
namespace {

std::wstring ToWideString(std::string_view value)
{
    std::wstring result;
    result.reserve(value.size());
    for (const char character : value) {
        result.push_back(static_cast<wchar_t>(character));
    }
    return result;
}

}

bool IsXamlResourcesProperty(std::string_view propertyName)
{
    return propertyName == "Resources";
}

XamlResourceResolver CreateXamlResourceResolver(
    NeoWPF::UI::Properties::DependencyObject& target,
    const XamlBuildContext& context)
{
    const auto* current = dynamic_cast<const IXamlResourceHost*>(&target);
    if (!current) {
        return {};
    }

    return [current, &context](std::string_view key, std::any& value) {
        if (current->TryFindXamlResource(std::string(key), value)) {
            return true;
        }

        for (auto it = context.scopes.rbegin(); it != context.scopes.rend(); ++it) {
            if (*it && (*it)->TryFindResource(std::string(key), value)) {
                return true;
            }
        }
        return false;
    };
}

bool BuildXamlResourceDictionary(
    const XamlNode& propertyNode,
    NeoWPF::UI::Properties::DependencyObject& owner,
    XamlNamescope* namescope,
    XamlBuildContext& context,
    const XamlObjectBuilder& buildObject)
{
    auto* ownerResourceHost = dynamic_cast<IXamlResourceHost*>(&owner);
    if (!ownerResourceHost || !buildObject) {
        return false;
    }

    const XamlNode* dictionary = &propertyNode;
    if (propertyNode.Children.size() == 1 && propertyNode.Children.front().Name == "ResourceDictionary") {
        dictionary = &propertyNode.Children.front();
    }

    std::unordered_set<std::string> localKeys;
    for (const auto& entry : dictionary->Children) {
        if (entry.Name.empty()) {
            continue;
        }
        if (entry.XamlKey.empty()) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_RESOURCE_KEY",
                L"Resource dictionary entry is missing x:Key",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            continue;
        }

        if (!localKeys.insert(entry.XamlKey).second) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_RESOURCE_KEY",
                L"Duplicate resource key: " + ToWideString(entry.XamlKey),
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            continue;
        }

        auto value = buildObject(entry, namescope, context);
        if (value) {
            ownerResourceHost->SetXamlResource(entry.XamlKey, value);
        }
    }
    return true;
}

}
