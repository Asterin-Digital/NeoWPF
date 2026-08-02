#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NeoWPF::UI::Properties {

enum class PropertyValueSource {
    Default,
    Local,
    Style,
    Expression,
};

enum class PropertyFlags : std::uint32_t {
    None = 0,
    AffectsMeasure = 1 << 0,
    AffectsArrange = 1 << 1,
    AffectsRender = 1 << 2,
    Inherits = 1 << 3,
};

inline PropertyFlags operator|(PropertyFlags left, PropertyFlags right) noexcept {
    return static_cast<PropertyFlags>(static_cast<std::underlying_type_t<PropertyFlags>>(left) |
                                      static_cast<std::underlying_type_t<PropertyFlags>>(right));
}

inline PropertyFlags operator&(PropertyFlags left, PropertyFlags right) noexcept {
    return static_cast<PropertyFlags>(static_cast<std::underlying_type_t<PropertyFlags>>(left) &
                                      static_cast<std::underlying_type_t<PropertyFlags>>(right));
}

inline PropertyFlags& operator|=(PropertyFlags& left, PropertyFlags right) noexcept {
    left = left | right;
    return left;
}

inline bool HasFlag(PropertyFlags flags, PropertyFlags test) noexcept {
    return (flags & test) != PropertyFlags::None;
}

class DependencyObject;
class DependencyPropertyRegistry;

struct DependencyPropertyChangedEventArgs {
    const class DependencyProperty* Property;
    std::any OldValue;
    std::any NewValue;
    PropertyValueSource Source;
};

using PropertyChangedCallback = std::function<void(DependencyObject*, const DependencyPropertyChangedEventArgs&)>;
using CoerceValueCallback = std::function<std::any(DependencyObject*, const std::any&)>;

struct PropertyMetadata {
    std::any DefaultValue;
    PropertyChangedCallback PropertyChangedCallback;
    CoerceValueCallback CoerceValueCallback;
    PropertyFlags Flags = PropertyFlags::None;
};

class DependencyProperty {
public:
    DependencyProperty(const std::string& name,
                       const std::type_index& propertyType,
                       const std::type_index& ownerType,
                       const PropertyMetadata& metadata);

    const std::type_index& GetPropertyType() const noexcept;
    const std::type_index& GetOwnerType() const noexcept;
    const PropertyMetadata& GetMetadata() const noexcept;
    const std::string& GetName() const noexcept;

    template<typename TOwner, typename TProperty>
    static const DependencyProperty& Register(const std::string& name, const PropertyMetadata& metadata = {});

private:
    std::type_index m_propertyType;
    std::type_index m_ownerType;
    std::string m_name;
    PropertyMetadata m_metadata;
};

class DependencyPropertyRegistry {
public:
    static DependencyPropertyRegistry& Get();

    const DependencyProperty& RegisterProperty(const std::string& name,
                                               const std::type_index& propertyType,
                                               const std::type_index& ownerType,
                                               const PropertyMetadata& metadata);

    template<typename TOwner>
    const DependencyProperty& RegisterProperty(const std::string& name,
                                               const std::type_index& propertyType,
                                               const PropertyMetadata& metadata);

    template<typename TOwner>
    void RegisterOwnerType();

    const DependencyProperty* FindProperty(const std::string& name,
                                           const std::type_index& ownerType) const;

    const DependencyProperty* FindProperty(const std::string& name,
                                           const DependencyObject& ownerInstance) const;

private:
    std::vector<std::unique_ptr<DependencyProperty>> m_properties;
    std::unordered_map<std::type_index, std::function<bool(const DependencyObject*)>> m_ownerTypeChecks;
};

template<typename TOwner, typename TProperty>
const DependencyProperty& DependencyProperty::Register(const std::string& name, const PropertyMetadata& metadata)
{
    auto& registry = DependencyPropertyRegistry::Get();
    return registry.RegisterProperty<TOwner>(name, typeid(TProperty), metadata);
}

template<typename TOwner>
const DependencyProperty& DependencyPropertyRegistry::RegisterProperty(const std::string& name,
                                                                       const std::type_index& propertyType,
                                                                       const PropertyMetadata& metadata)
{
    RegisterOwnerType<TOwner>();
    return RegisterProperty(name, propertyType, typeid(TOwner), metadata);
}

template<typename TOwner>
void DependencyPropertyRegistry::RegisterOwnerType()
{
    const std::type_index ownerType = typeid(TOwner);
    if (m_ownerTypeChecks.find(ownerType) == m_ownerTypeChecks.end()) {
        m_ownerTypeChecks[ownerType] = [](const DependencyObject* object) {
            return dynamic_cast<const TOwner*>(object) != nullptr;
        };
    }
}

}
