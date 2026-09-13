/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <NeoWPF/UI/Properties/TypeConverter.h>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NeoWPF::UI::Properties {

/*
    @summary
    Describes where a dependency property value originated.
*/
enum class PropertyValueSource {
    Default,
    Local,
    Style,
    Trigger,
    Expression,
    Animation,
};

/*
    @summary
    Flags that describe how a dependency property affects layout and rendering.
*/
enum class PropertyFlags : std::uint32_t {
    None = 0,
    AffectsMeasure = 1 << 0,
    AffectsArrange = 1 << 1,
    AffectsRender = 1 << 2,
    Inherits = 1 << 3,
};

/*
    @summary
    Combines two property flags.
*/
inline PropertyFlags operator|(PropertyFlags left, PropertyFlags right) noexcept {
    return static_cast<PropertyFlags>(static_cast<std::underlying_type_t<PropertyFlags>>(left) |
                                      static_cast<std::underlying_type_t<PropertyFlags>>(right));
}

/*
    @summary
    Intersects two property flag sets.
*/
inline PropertyFlags operator&(PropertyFlags left, PropertyFlags right) noexcept {
    return static_cast<PropertyFlags>(static_cast<std::underlying_type_t<PropertyFlags>>(left) &
                                      static_cast<std::underlying_type_t<PropertyFlags>>(right));
}

/*
    @summary
    Appends one set of flags onto another.
*/
inline PropertyFlags& operator|=(PropertyFlags& left, PropertyFlags right) noexcept {
    left = left | right;
    return left;
}

/*
    @summary
    Checks whether a specific property flag is present.
*/
inline bool HasFlag(PropertyFlags flags, PropertyFlags test) noexcept {
    return (flags & test) != PropertyFlags::None;
}

class DependencyObject;
class DependencyPropertyRegistry;
}

namespace NeoWPF::UI::Markup::Xaml {
struct XamlPropertyBinding;
}

namespace NeoWPF::UI::Properties {

/*
    @summary
    Carries the old and new values when a dependency property changes.
*/
struct DependencyPropertyChangedEventArgs {
    const class DependencyProperty* Property;
    std::any OldValue;
    std::any NewValue;
    PropertyValueSource Source;
};

using PropertyChangedCallback = std::function<void(DependencyObject*, const DependencyPropertyChangedEventArgs&)>;
using CoerceValueCallback = std::function<std::any(DependencyObject*, const std::any&)>;
using PropertyChangedCallbackFunction = PropertyChangedCallback;
using CoerceValueCallbackFunction = CoerceValueCallback;
using XamlValueParser = std::function<bool(const std::string&, std::any&)>;

/*
    @summary
    Stores the default value and callbacks used by a dependency property.
*/
struct PropertyMetadata {
    std::any DefaultValue;
    PropertyChangedCallbackFunction PropertyChangedCallback;
    CoerceValueCallbackFunction CoerceValueCallback;
    PropertyFlags Flags = PropertyFlags::None;
    XamlValueParser ValueParser;
    std::shared_ptr<const TypeConverter> Converter;
};

bool ValuesEqual(const std::any& left, const std::any& right) noexcept;
bool ValuesEqual(const DependencyObject& owner,
                 const DependencyProperty& property,
                 const std::any& expectedValue);

/*
    @summary
    Represents a dependency property definition registered with the property system.

    @details
    DependencyProperty describes the property name, value type, owner type, and metadata used when values are stored and propagated.
*/
class DependencyProperty {
public:
    /*
        @summary
        Creates a dependency property definition.

        @param name
        The property name.

        @param propertyType
        The CLR-like runtime type of the property values.

        @param ownerType
        The type that owns the property.

        @param metadata
        The default value and callbacks associated with the property.

        @param attachedTargetType
        The target type an attached property applies to, if this is an attached property.

        @param isAttached
        True if the property is an attached property rather than an owner-bound property.
    */
    DependencyProperty(const std::string& name,
                       const std::type_index& propertyType,
                       const std::type_index& ownerType,
                       const PropertyMetadata& metadata,
                       const std::type_index& attachedTargetType = std::type_index(typeid(void)),
                       bool isAttached = false);

    /*
        @summary
        Returns the runtime type of the property values.
    */
    const std::type_index& GetPropertyType() const noexcept;

    /*
        @summary
        Returns the type that owns the property.
    */
    const std::type_index& GetOwnerType() const noexcept;

    /*
        @summary
        Returns the metadata associated with the property.
    */
    const PropertyMetadata& GetMetadata() const noexcept;

    /*
        @summary
        Returns the property name.
    */
    const std::string& GetName() const noexcept;

    /*
        @summary
        Returns true when the property is attached to another target type.
    */
    bool IsAttached() const noexcept;

    /*
        @summary
        Returns the type this attached property applies to.
    */
    const std::type_index& GetAttachedTargetType() const noexcept;

    /*
        @summary
        Returns XAML property binding metadata for this property, if registered.

        @return
        Pointer to XamlPropertyBinding if this property has XAML binding metadata; nullptr otherwise.
    */
    const NeoWPF::UI::Markup::Xaml::XamlPropertyBinding* GetXamlBinding() const noexcept;

    /*
        @summary
        Tries to parse a XAML attribute string into the value type for this property.
    */
    bool TryParseValue(const std::string& rawValue, std::any& parsedValue) const;

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const;

    /*
        @summary
        Registers a dependency property for a specific owner type.

        @tparam TOwner
        The owner of the property.

        @tparam TProperty
        The property value type.

        @param name
        The name of the dependency property.

        @param metadata
        Optional default value and callback metadata.
    */
    template<typename TOwner, typename TProperty>
    static const DependencyProperty& Register(const std::string& name, const PropertyMetadata& metadata = {});

    /*
        @summary
        Registers an attached dependency property for a specific owner and target type.
    */
    template<typename TOwner, typename TTarget, typename TProperty>
    static const DependencyProperty& RegisterAttached(const std::string& name, const PropertyMetadata& metadata = {});

private:
    std::type_index m_propertyType;
    std::type_index m_ownerType;
    std::string m_name;
    PropertyMetadata m_metadata;
    std::type_index m_attachedTargetType = std::type_index(typeid(void));
    bool m_isAttached = false;
};

/*
    @summary
    Maintains the registry of dependency properties for the application.
*/
class DependencyPropertyRegistry {
public:
    /*
        @summary
        Returns the singleton registry instance.
    */
    static DependencyPropertyRegistry& Get();

    /*
        @summary
        Registers a dependency property definition in the registry.

        @param name
        The property name.

        @param propertyType
        The runtime value type for the property.

        @param ownerType
        The owner type for the property.

        @param metadata
        The metadata to associate with the property.
    */
    const DependencyProperty& RegisterProperty(const std::string& name,
                                               const std::type_index& propertyType,
                                               const std::type_index& ownerType,
                                               const PropertyMetadata& metadata);

    /*
        @summary
        Registers a dependency property for a given owner type.

        @tparam TOwner
        The owner type.

        @param name
        The property name.

        @param propertyType
        The runtime value type for the property.

        @param metadata
        The metadata to associate with the property.
    */
    template<typename TOwner>
    const DependencyProperty& RegisterProperty(const std::string& name,
                                               const std::type_index& propertyType,
                                               const PropertyMetadata& metadata);

    /*
        @summary
        Registers an attached dependency property definition.
    */
    const DependencyProperty& RegisterAttachedProperty(const std::string& name,
                                                       const std::type_index& propertyType,
                                                       const std::type_index& ownerType,
                                                       const std::type_index& attachedTargetType,
                                                       const PropertyMetadata& metadata);

    /*
        @summary
        Registers the owner type so the registry can recognize instances of it.

        @tparam TOwner
        The owner type to register.
    */
    template<typename TOwner>
    void RegisterOwnerType();

    /*
        @summary
        Finds a property by name for a specific owner type.

        @param name
        The property name to look up.

        @param ownerType
        The owner type to match.
    */
    const DependencyProperty* FindProperty(const std::string& name,
                                           const std::type_index& ownerType) const;

    /*
        @summary
        Finds a property by name for a specific object instance.

        @param name
        The property name to look up.

        @param ownerInstance
        The object instance that should own the property.
    */
    const DependencyProperty* FindProperty(const std::string& name,
                                           const DependencyObject& ownerInstance) const;

    /*
        @summary
        Finds an attached property by name for a target type and owner type.
    */
    const DependencyProperty* FindAttachedProperty(const std::string& name,
                                                  const std::type_index& targetType,
                                                  const std::type_index& ownerType) const;

    /*
        @summary
        Finds an attached property by name for a target instance and owner type.
    */
    const DependencyProperty* FindAttachedProperty(const std::string& name,
                                                  const DependencyObject& targetInstance,
                                                  const std::type_index& ownerType) const;

    std::vector<const DependencyProperty*> GetPropertiesFor(const DependencyObject& ownerInstance) const;

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

template<typename TOwner, typename TTarget, typename TProperty>
const DependencyProperty& DependencyProperty::RegisterAttached(const std::string& name, const PropertyMetadata& metadata)
{
    auto& registry = DependencyPropertyRegistry::Get();
    registry.RegisterOwnerType<TTarget>();
    return registry.RegisterAttachedProperty(name, typeid(TProperty), typeid(TOwner), typeid(TTarget), metadata);
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
