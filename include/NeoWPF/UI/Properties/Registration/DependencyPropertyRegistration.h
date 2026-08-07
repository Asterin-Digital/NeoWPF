#pragma once

#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <any>
#include <string>

namespace NeoWPF::UI::Properties {

    template<typename TOwner, typename TProperty>
    const DependencyProperty& RegisterProperty(const std::string& name, const PropertyMetadata& metadata = {}) {
        return DependencyProperty::Register<TOwner, TProperty>(name, metadata);
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
                        if constexpr (std::is_same_v<TProperty, NeoWPF::UI::Color>) {
                            if (args.NewValue.type() == typeid(std::uint32_t)) {
                                const auto value = std::any_cast<std::uint32_t>(args.NewValue);
                                std::invoke(setter, owner, NeoWPF::UI::Color::FromArgb(value));
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

    void RegisterUIElementProperties();
    void RegisterFrameworkElementProperties();
    void RegisterControlProperties();
    void RegisterButtonProperties();
    void RegisterCheckboxProperties();
    void RegisterItemsControlProperties();
    void RegisterScrollBarProperties();
    void RegisterTextBlockProperties();
    void RegisterTextBoxProperties();
    void RegisterImageProperties();
    void RegisterBorderProperties();
    void RegisterPanelProperties();

    void RegisterCoreControlProperties();

}
