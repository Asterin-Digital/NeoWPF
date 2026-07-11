#include <NeoWPF/UI/Properties/Property/PropertyRegistry.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Properties {
    std::unordered_map<std::type_index, std::unique_ptr<PropertyRegistry>> PropertyRegistry::s_registries;

    PropertyRegistry& PropertyRegistry::Get(const std::type_index& type) {
        auto it = s_registries.find(type);
        if (it != s_registries.end()) {
            return *it->second;
        }
        return *s_registries.emplace(type, std::make_unique<PropertyRegistry>()).first->second;
    }

    void PropertyRegistry::RegisterProperty(const std::string& name, const PropertyInfo& info) {
        auto result = m_properties.emplace(name, info);
        if (!result.second) {
            result.first->second = info;
        }
    }

    bool PropertyRegistry::SetPropertyValue(UIElement* element, const std::string& name, const std::any& value, PropertySource source) {
        auto it = m_properties.find(name);
        if (it == m_properties.end()) {
            return false;
        }

        if (source == PropertySource::Style && element->IsPropertyLocal(name)) {
            return true;
        }

        it->second.Setter(element, value);
        element->SetPropertySource(name, source);
        return true;
    }

    std::any PropertyRegistry::GetPropertyValue(UIElement* element, const std::string& name) const {
        auto it = m_properties.find(name);
        if (it == m_properties.end()) {
            return std::any();
        }

        try {
            return it->second.Getter(element);
        } catch (...) {
            return std::any();
        }
    }

    std::vector<PropertyInfo> PropertyRegistry::GetProperties() const {
        std::vector<PropertyInfo> result;
        for (const auto& pair : m_properties) {
            result.push_back(pair.second);
        }
        return result;
    }

    bool PropertyRegistry::HasProperty(const std::string& name) const {
        return m_properties.find(name) != m_properties.end();
    }
}
