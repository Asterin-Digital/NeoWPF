#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>

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

}
