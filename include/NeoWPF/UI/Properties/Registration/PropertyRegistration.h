#pragma once

#include <NeoWPF/UI/Properties/Property/PropertyRegistry.h>
#include <NeoWPF/UI/Properties/Property/PropertyInfo.h>
#include <any>
#include <functional>

namespace NeoWPF::UI {
    class UIElement;
}

namespace NeoWPF::UI::Properties {
    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        PropertyType (ControlType::*getter)() const,
        void (ControlType::*setter)(PropertyType))
    {
        auto& registry = PropertyRegistry::Get(typeid(ControlType));
        registry.RegisterProperty(name, PropertyInfo{
            name,
            typeid(PropertyType),
            [getter, setter](UIElement* element, const std::any& value) {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    (ctrl->*setter)(std::any_cast<PropertyType>(value));
                }
            },
            [getter](UIElement* element) -> std::any {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    return std::any((ctrl->*getter)());
                }
                return std::any();
            }
        });
    }

    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        PropertyType (ControlType::*getter)() const,
        void (ControlType::*setter)(const PropertyType&))
    {
        auto& registry = PropertyRegistry::Get(typeid(ControlType));
        registry.RegisterProperty(name, PropertyInfo{
            name,
            typeid(PropertyType),
            [getter, setter](UIElement* element, const std::any& value) {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    (ctrl->*setter)(std::any_cast<PropertyType>(value));
                }
            },
            [getter](UIElement* element) -> std::any {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    return std::any((ctrl->*getter)());
                }
                return std::any();
            }
        });
    }

    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        const PropertyType& (ControlType::*getter)() const,
        void (ControlType::*setter)(const PropertyType&))
    {
        auto& registry = PropertyRegistry::Get(typeid(ControlType));
        registry.RegisterProperty(name, PropertyInfo{
            name,
            typeid(PropertyType),
            [getter, setter](UIElement* element, const std::any& value) {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    (ctrl->*setter)(std::any_cast<PropertyType>(value));
                }
            },
            [getter](UIElement* element) -> std::any {
                if (auto ctrl = dynamic_cast<ControlType*>(element)) {
                    return std::any((ctrl->*getter)());
                }
                return std::any();
            }
        });
    }

    void RegisterCoreControlProperties();

    void RegisterUIElementProperties();
    void RegisterFrameworkElementProperties();
    void RegisterButtonProperties();
    void RegisterTextBlockProperties();
    void RegisterTextBoxProperties();
    void RegisterBorderProperties();
    void RegisterPanelProperties();
    void RegisterScrollBarProperties();
}
