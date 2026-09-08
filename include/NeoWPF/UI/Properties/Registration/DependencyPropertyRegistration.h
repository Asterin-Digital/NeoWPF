#pragma once

#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <any>
#include <cstring>
#include <string>
#include <type_traits>

namespace NeoWPF::UI::Properties {

    template<typename TOwner, typename TProperty>
    const DependencyProperty& RegisterProperty(const std::string& name, const PropertyMetadata& metadata = {}) {
        return DependencyProperty::Register<TOwner, TProperty>(name, metadata);
    }

    template<typename TOwner, typename TTarget, typename TProperty>
    const DependencyProperty& RegisterAttachedProperty(const std::string& name, const PropertyMetadata& metadata = {}) {
        return DependencyProperty::RegisterAttached<TOwner, TTarget, TProperty>(name, metadata);
    }

    template<typename TOwner, typename TProperty, typename TGetter, typename TSetter>
    const DependencyProperty* RegisterStaticPropertyHandle(const char* name,
                                                          TGetter getter,
                                                          TSetter setter,
                                                          const PropertyMetadata& metadata = {})
    {
        using DecayedProperty = std::remove_cvref_t<TProperty>;

        static_assert(std::is_member_function_pointer_v<TGetter> || std::is_function_v<TGetter>,
                      "getter must be a member function pointer or function pointer");
        static_assert(std::is_member_function_pointer_v<TSetter> || std::is_function_v<TSetter>,
                      "setter must be a member function pointer or function pointer");

        static_assert(std::is_same_v<std::remove_cvref_t<std::invoke_result_t<TGetter, TOwner&>>, DecayedProperty>,
                      "getter return type must match the property value type");

        static_assert(
            std::is_invocable_r_v<void, TSetter, TOwner&, TProperty> ||
            std::is_invocable_r_v<void, TSetter, TOwner&, const TProperty&> ||
            std::is_invocable_r_v<void, TSetter, TOwner&, DecayedProperty> ||
            std::is_invocable_r_v<void, TSetter, TOwner&, const DecayedProperty&>,
            "setter must accept (Owner&, PropertyType) or const PropertyType&");

        if (const auto* existing = DependencyPropertyRegistry::Get().FindProperty(name, typeid(TOwner))) {
            return existing;
        }

        PropertyMetadata registrationMetadata = metadata;
        if (!registrationMetadata.PropertyChangedCallback) {
            registrationMetadata.PropertyChangedCallback = [setter](DependencyObject* sender, const DependencyPropertyChangedEventArgs& args) {
                if (auto owner = dynamic_cast<TOwner*>(sender)) {
                    try {
                        std::invoke(setter, owner, std::any_cast<TProperty>(args.NewValue));
                    } catch (const std::bad_any_cast&) {
                        if constexpr (std::is_same_v<TProperty, ::NeoWPF::UI::Color>) {
                            if (args.NewValue.type() == typeid(std::uint32_t)) {
                                const auto value = std::any_cast<std::uint32_t>(args.NewValue);
                                std::invoke(setter, owner, ::NeoWPF::UI::Color::FromArgb(value));
                            }
                        }
                    }
                }
            };
        }

        (void)getter;
        return &DependencyProperty::Register<TOwner, TProperty>(name, registrationMetadata);
    }

    template<typename TOwner, typename TProperty, typename TGetter, typename TSetter>
    const DependencyProperty& RegisterProperty(const std::string& name,
                                               TGetter getter,
                                               TSetter setter,
                                               const PropertyMetadata& metadata = {})
    {
        PropertyMetadata registrationMetadata = metadata;
        if (!registrationMetadata.PropertyChangedCallback) {
            registrationMetadata.PropertyChangedCallback = [setter](DependencyObject* sender, const DependencyPropertyChangedEventArgs& args) {
                if (auto owner = dynamic_cast<TOwner*>(sender)) {
                    try {
                        std::invoke(setter, owner, std::any_cast<TProperty>(args.NewValue));
                    } catch (const std::bad_any_cast&) {
                        if constexpr (std::is_same_v<TProperty, ::NeoWPF::UI::Color>) {
                            if (args.NewValue.type() == typeid(std::uint32_t)) {
                                const auto value = std::any_cast<std::uint32_t>(args.NewValue);
                                std::invoke(setter, owner, ::NeoWPF::UI::Color::FromArgb(value));
                            }
                        }
                    }
                }
            };
        }
        (void)getter;
        return RegisterProperty<TOwner, TProperty>(name, registrationMetadata);
    }

    inline std::wstring CoerceToWString(const std::any& value)
    {
        if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? L"True" : L"False";
        }

        if (value.type() == typeid(int)) {
            return std::to_wstring(std::any_cast<int>(value));
        }

        if (value.type() == typeid(float)) {
            return std::to_wstring(std::any_cast<float>(value));
        }

        if (value.type() == typeid(double)) {
            return std::to_wstring(std::any_cast<double>(value));
        }

        if (value.type() == typeid(std::wstring)) {
            return std::any_cast<std::wstring>(value);
        }

        if (value.type() == typeid(std::string)) {
            const auto textValue = std::any_cast<std::string>(value);
            return std::wstring(textValue.begin(), textValue.end());
        }

        if (value.type() == typeid(const wchar_t*)) {
            const auto textValue = std::any_cast<const wchar_t*>(value);
            return textValue ? std::wstring(textValue) : std::wstring();
        }

        if (value.type() == typeid(const char*)) {
            const auto textValue = std::any_cast<const char*>(value);
            if (!textValue) {
                return std::wstring();
            }
            return std::wstring(textValue, textValue + std::strlen(textValue));
        }

        return std::wstring();
    }

    #define DEFINE_STATIC_PROPERTY_HANDLE(OwnerType, PropertyName, PropertyType, GetterExpression, SetterExpression, MetadataFactory) \
        [[nodiscard]] const NeoWPF::UI::Properties::DependencyProperty& OwnerType::PropertyName##Property() { \
            static const auto* property = []() -> const NeoWPF::UI::Properties::DependencyProperty* { \
                using GetterType = decltype(GetterExpression); \
                using SetterType = decltype(SetterExpression); \
                static_assert(std::is_member_function_pointer_v<GetterType> || std::is_function_v<GetterType>, \
                              "getter must be a member function pointer or function pointer"); \
                static_assert(std::is_member_function_pointer_v<SetterType> || std::is_function_v<SetterType>, \
                              "setter must be a member function pointer or function pointer"); \
                static_assert(std::is_same_v<std::remove_cvref_t<std::invoke_result_t<GetterType, OwnerType&>>, std::remove_cvref_t<PropertyType>>, \
                              "getter return type must match the dependency property type"); \
                static_assert(std::is_invocable_r_v<void, SetterType, OwnerType&, PropertyType> || \
                                  std::is_invocable_r_v<void, SetterType, OwnerType&, const PropertyType&> || \
                                  std::is_invocable_r_v<void, SetterType, OwnerType&, std::remove_cvref_t<PropertyType>> || \
                                  std::is_invocable_r_v<void, SetterType, OwnerType&, const std::remove_cvref_t<PropertyType>&>, \
                              "setter must accept the owning type and the property value type"); \
                const auto metadata = (MetadataFactory)(); \
                return NeoWPF::UI::Properties::RegisterStaticPropertyHandle<OwnerType, PropertyType>( \
                    #PropertyName, GetterExpression, SetterExpression, metadata); \
            }(); \
            return *property; \
        }

    #define DEFINE_STATIC_ATTACHED_PROPERTY_HANDLE(OwnerType, PropertyName, AttachedTargetType, PropertyType, GetterExpression, SetterExpression, MetadataFactory) \
        [[nodiscard]] const NeoWPF::UI::Properties::DependencyProperty& OwnerType::PropertyName##Property() { \
            static const auto* property = []() -> const NeoWPF::UI::Properties::DependencyProperty* { \
                const auto metadata = (MetadataFactory)(); \
                return &NeoWPF::UI::Properties::RegisterAttachedProperty<OwnerType, AttachedTargetType, PropertyType>(#PropertyName, metadata); \
            }(); \
            return *property; \
        }

    void RegisterUIElementProperties();
    void RegisterFrameworkElementProperties();
    void RegisterControlProperties();
    void RegisterContentControlProperties();
    void RegisterWindowProperties();
    void RegisterApplicationProperties();
    void RegisterButtonProperties();
    void RegisterRangeBaseProperties();
    void RegisterThumbProperties();
    void RegisterCheckboxProperties();
    void RegisterItemsControlProperties();
    void RegisterScrollBarProperties();
    void RegisterTextBlockProperties();
    void RegisterTextBoxProperties();
    void RegisterImageProperties();
    void RegisterBorderProperties();
    void RegisterPanelProperties();
    void RegisterGridProperties();
    void RegisterRowDefinitionProperties();
    void RegisterColumnDefinitionProperties();
    void RegisterScrollViewerProperties();
    void RegisterStyleProperties();

    void RegisterCoreControlProperties();

}
