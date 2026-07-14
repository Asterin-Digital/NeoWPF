#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterUIElementProperties() {
        RegisterProperty<UI::UIElement, bool>(
            "Visibility",
            &UI::UIElement::IsVisible,
            &UI::UIElement::SetVisibility);

        RegisterProperty<UI::UIElement, bool>(
            "IsEnabled",
            &UI::UIElement::IsEnabled,
            &UI::UIElement::SetEnabled);

        RegisterProperty<UI::UIElement, float>(
            "Opacity",
            &UI::UIElement::GetOpacity,
            &UI::UIElement::SetOpacity);
    }
}
