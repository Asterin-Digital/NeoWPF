#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <any>
#include <memory>
#include "PropertyInfo.h"

namespace NeoWPF::UI {
    class UIElement;
}

namespace NeoWPF::UI::Properties {
    class PropertyRegistry {
    public:
        static PropertyRegistry& Get(const std::type_index& type);
        
        void RegisterProperty(const std::string& name, const PropertyInfo& info);
        
        bool SetPropertyValue(UIElement* element, const std::string& name, const std::any& value);
        
        std::any GetPropertyValue(UIElement* element, const std::string& name) const;
        
        std::vector<PropertyInfo> GetProperties() const;
        
        bool HasProperty(const std::string& name) const;
        
    private:
        PropertyRegistry() = default;
        
        std::unordered_map<std::string, PropertyInfo> m_properties;
        
        static std::unordered_map<std::type_index, std::unique_ptr<PropertyRegistry>> s_registries;
    };
}
