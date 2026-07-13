#pragma once

#include <NeoWPF/UI/Properties/Property/PropertyRegistry.h>
#include <NeoWPF/UI/Properties/Property/PropertyInfo.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <any>
#include <cstdint>
#include <type_traits>
#include <string>

namespace NeoWPF::UI {
    class UIElement;
    class ItemsControl;
}

namespace NeoWPF::UI::Properties {
    template<typename PropertyType>
    PropertyType PropertyValueFromAny(const std::any& value);

    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        PropertyType (ControlType::*getter)() const,
        void (ControlType::*setter)(PropertyType));

    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        PropertyType (ControlType::*getter)() const,
        void (ControlType::*setter)(const PropertyType&));

    template<typename ControlType, typename PropertyType>
    void RegisterProperty(
        const std::string& name,
        const PropertyType& (ControlType::*getter)() const,
        void (ControlType::*setter)(const PropertyType&));

    void RegisterCoreControlProperties();

    void RegisterUIElementProperties();
    void RegisterFrameworkElementProperties();
    void RegisterButtonProperties();
    void RegisterItemsControlProperties();
    void RegisterTextBlockProperties();
    void RegisterTextBoxProperties();
    void RegisterBorderProperties();
    void RegisterPanelProperties();
    void RegisterScrollBarProperties();
}
