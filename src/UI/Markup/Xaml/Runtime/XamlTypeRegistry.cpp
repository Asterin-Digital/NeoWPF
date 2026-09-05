#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>

#include <utility>

namespace NeoWPF::UI::Markup::Xaml {

XamlTypeRegistry& XamlTypeRegistry::Get() {
    static XamlTypeRegistry instance;
    return instance;
}

void XamlTypeRegistry::Register(std::string_view tagName, const XamlTypeInfo& info) {
    const auto key = std::string(tagName);
    
    // Avoid duplicate registration (keep first one)
    if (m_types.find(key) != m_types.end()) {
        return;
    }
    
    m_types.emplace(key, info);
}

const XamlTypeInfo* XamlTypeRegistry::Find(std::string_view tagName) const {
    const auto it = m_types.find(std::string(tagName));
    if (it != m_types.end()) {
        return &it->second;
    }
    return nullptr;
}

const XamlTypeInfo* XamlTypeRegistry::FindByType(std::type_index type) const {
    for (const auto& [tagName, info] : m_types) {
        if (info.Type == type) {
            return &info;
        }
    }
    return nullptr;
}

std::vector<RegisteredXamlType> XamlTypeRegistry::GetRegisteredTypes() const
{
    std::vector<RegisteredXamlType> result;
    result.reserve(m_types.size());
    for (const auto& [tagName, info] : m_types) {
        result.push_back({tagName, info});
    }
    return result;
}

bool ValidateXamlRegistrations()
{
    bool valid = true;
    const auto registeredTypes = XamlTypeRegistry::Get().GetRegisteredTypes();
    auto& propertyRegistry = NeoWPF::UI::Properties::DependencyPropertyRegistry::Get();

    for (const auto& registeredType : registeredTypes) {
        if (!registeredType.Info.Factory) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_REGISTRATION_INVALID",
                L"XAML type '" + std::wstring(registeredType.TagName.begin(), registeredType.TagName.end()) +
                    L"' has no factory.",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            valid = false;
            continue;
        }

        const bool hasCppTypeName = !registeredType.Info.CppTypeName.empty();
        const bool hasHeaderPath = !registeredType.Info.HeaderPath.empty();
        if (!hasCppTypeName || !hasHeaderPath) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_REGISTRATION_INVALID",
                L"XAML type '" + std::wstring(registeredType.TagName.begin(), registeredType.TagName.end()) +
                    L"' must provide both C++ type and header metadata.",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            valid = false;
        }

        const auto object = registeredType.Info.Factory();
        if (!object) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_REGISTRATION_INVALID",
                L"XAML type '" + std::wstring(registeredType.TagName.begin(), registeredType.TagName.end()) +
                    L"' factory returned null.",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            valid = false;
            continue;
        }

        if (propertyRegistry.GetPropertiesFor(*object).empty()) {
            NeoWPF::Diagnostics::QueueDeveloperWarning(
                L"XAML_REGISTRATION_INVALID",
                L"XAML type '" + std::wstring(registeredType.TagName.begin(), registeredType.TagName.end()) +
                    L"' has no registered dependency properties.",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
            valid = false;
        }

        if (!registeredType.Info.ContentPropertyName.empty()) {
            const auto* property = object->FindDependencyProperty(registeredType.Info.ContentPropertyName);
            const auto* binding = property ? property->GetXamlBinding() : nullptr;
            const bool usesVisualContentFallback = !property &&
                dynamic_cast<NeoWPF::UI::Visual*>(object.get()) != nullptr;
            if (!usesVisualContentFallback &&
                (!binding || (binding->Kind != XamlPropertyKind::Element &&
                              binding->Kind != XamlPropertyKind::Collection))) {
                NeoWPF::Diagnostics::QueueDeveloperWarning(
                    L"XAML_REGISTRATION_INVALID",
                    L"XAML type '" + std::wstring(registeredType.TagName.begin(), registeredType.TagName.end()) +
                        L"' content property '" +
                        std::wstring(registeredType.Info.ContentPropertyName.begin(), registeredType.Info.ContentPropertyName.end()) +
                        L"' has no element or collection binding.",
                    NeoWPF::Diagnostics::DeveloperWarningSeverity::Error);
                valid = false;
            }
        }
    }

    return valid;
}

}
