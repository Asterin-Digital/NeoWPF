#include <NeoWPF/UI/Markup/Xaml/XamlPropertyBinding.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI::Markup::Xaml {

XamlPropertyBindingRegistry& XamlPropertyBindingRegistry::Get() {
    static XamlPropertyBindingRegistry instance;
    return instance;
}

void XamlPropertyBindingRegistry::Register(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    const XamlPropertyBinding& binding)
{
    const auto key = reinterpret_cast<std::uintptr_t>(&property);

    // keep the first one to avoid dupes
    if (m_bindings.find(key) != m_bindings.end()) {
        return;
    }

    m_bindings.emplace(key, binding);
}

const XamlPropertyBinding* XamlPropertyBindingRegistry::Find(
    const NeoWPF::UI::Properties::DependencyProperty& property) const
{
    const auto key = reinterpret_cast<std::uintptr_t>(&property);
    const auto it = m_bindings.find(key);
    if (it != m_bindings.end()) {
        return &it->second;
    }
    return nullptr;
}

}
