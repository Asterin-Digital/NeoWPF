#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <any>
#include <cstdint>
#include <type_traits>
#include <string>

namespace NeoWPF::UI::Properties {
    template<typename PropertyType>
    PropertyType PropertyValueFromAny(const std::any& value)
    {
        if constexpr (std::is_same_v<PropertyType, Color>) {
            if (value.type() == typeid(std::uint32_t)) {
                return Color::FromArgb(std::any_cast<std::uint32_t>(value));
            }
        }

        return std::any_cast<PropertyType>(value);
    }

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
                    (ctrl->*setter)(PropertyValueFromAny<PropertyType>(value));
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
                    (ctrl->*setter)(PropertyValueFromAny<PropertyType>(value));
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
                    (ctrl->*setter)(PropertyValueFromAny<PropertyType>(value));
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

    void RegisterCoreControlProperties() {
        RegisterUIElementProperties();
        RegisterFrameworkElementProperties();
        RegisterButtonProperties();
        RegisterTextBlockProperties();
        RegisterTextBoxProperties();
        RegisterBorderProperties();
        RegisterPanelProperties();
        RegisterScrollBarProperties();
    }

    // i really really hate c++ templates :<
    template void RegisterProperty<UI::UIElement, bool>(
        const std::string&,
        bool (UI::UIElement::*)() const,
        void (UI::UIElement::*)(bool));

    template void RegisterProperty<UI::UIElement, float>(
        const std::string&,
        float (UI::UIElement::*)() const,
        void (UI::UIElement::*)(float));

    template void RegisterProperty<UI::FrameworkElement, int>(
        const std::string&,
        int (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(int));

    template void RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>(
        const std::string&,
        UI::LayoutSizeMode (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(UI::LayoutSizeMode));

    template void RegisterProperty<UI::FrameworkElement, bool>(
        const std::string&,
        bool (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(bool));

    template void RegisterProperty<UI::FrameworkElement, UI::HorizontalAlignment>(
        const std::string&,
        UI::HorizontalAlignment (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(UI::HorizontalAlignment));

    template void RegisterProperty<UI::FrameworkElement, UI::VerticalAlignment>(
        const std::string&,
        UI::VerticalAlignment (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(UI::VerticalAlignment));

    template void RegisterProperty<UI::FrameworkElement, UI::CornerRadius>(
        const std::string&,
        UI::CornerRadius (UI::FrameworkElement::*)() const,
        void (UI::FrameworkElement::*)(UI::CornerRadius));

    template void RegisterProperty<UI::Button, std::wstring>(
        const std::string&,
        const std::wstring& (UI::Button::*)() const,
        void (UI::Button::*)(const std::wstring&));

    template void RegisterProperty<UI::Button, UI::Color>(
        const std::string&,
        UI::Color (UI::Button::*)() const,
        void (UI::Button::*)(UI::Color));

    template void RegisterProperty<UI::Button, int>(
        const std::string&,
        int (UI::Button::*)() const,
        void (UI::Button::*)(int));

    template void RegisterProperty<UI::TextBlock, std::wstring>(
        const std::string&,
        const std::wstring& (UI::TextBlock::*)() const,
        void (UI::TextBlock::*)(const std::wstring&));

    template void RegisterProperty<UI::TextBlock, UI::Color>(
        const std::string&,
        UI::Color (UI::TextBlock::*)() const,
        void (UI::TextBlock::*)(UI::Color));

    template void RegisterProperty<UI::TextBlock, int>(
        const std::string&,
        int (UI::TextBlock::*)() const,
        void (UI::TextBlock::*)(int));

    template void RegisterProperty<UI::TextBox, std::wstring>(
        const std::string&,
        const std::wstring& (UI::TextBox::*)() const,
        void (UI::TextBox::*)(const std::wstring&));

    template void RegisterProperty<UI::TextBox, UI::Color>(
        const std::string&,
        UI::Color (UI::TextBox::*)() const,
        void (UI::TextBox::*)(UI::Color));

    template void RegisterProperty<UI::TextBox, int>(
        const std::string&,
        int (UI::TextBox::*)() const,
        void (UI::TextBox::*)(int));

    template void RegisterProperty<UI::Border, int>(
        const std::string&,
        int (UI::Border::*)() const,
        void (UI::Border::*)(int));

    template void RegisterProperty<UI::Border, UI::Color>(
        const std::string&,
        UI::Color (UI::Border::*)() const,
        void (UI::Border::*)(UI::Color));

    template void RegisterProperty<UI::Panel, UI::Color>(
        const std::string&,
        UI::Color (UI::Panel::*)() const,
        void (UI::Panel::*)(UI::Color));

    template void RegisterProperty<UI::ScrollBar, UI::Color>(
        const std::string&,
        UI::Color (UI::ScrollBar::*)() const,
        void (UI::ScrollBar::*)(UI::Color));

    template void RegisterProperty<UI::ScrollBar, int>(
        const std::string&,
        int (UI::ScrollBar::*)() const,
        void (UI::ScrollBar::*)(int));

}

