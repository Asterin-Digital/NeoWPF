#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterFrameworkElementProperties() {
        RegisterProperty<UI::FrameworkElement, int>(
            "Width",
            &UI::FrameworkElement::GetWidth,
            &UI::FrameworkElement::SetWidth);

        RegisterProperty<UI::FrameworkElement, int>(
            "Height",
            &UI::FrameworkElement::GetHeight,
            &UI::FrameworkElement::SetHeight);

        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>(
            "WidthMode",
            &UI::FrameworkElement::GetWidthMode,
            &UI::FrameworkElement::SetWidthMode);

        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>(
            "HeightMode",
            &UI::FrameworkElement::GetHeightMode,
            &UI::FrameworkElement::SetHeightMode);

        RegisterProperty<UI::FrameworkElement, bool>(
            "ClipToBounds",
            &UI::FrameworkElement::IsClipToBounds,
            &UI::FrameworkElement::SetClipToBounds);

        RegisterProperty<UI::FrameworkElement, UI::HorizontalAlignment>(
            "HorizontalAlignment",
            &UI::FrameworkElement::GetHorizontalAlignment,
            &UI::FrameworkElement::SetHorizontalAlignment);

        RegisterProperty<UI::FrameworkElement, UI::VerticalAlignment>(
            "VerticalAlignment",
            &UI::FrameworkElement::GetVerticalAlignment,
            &UI::FrameworkElement::SetVerticalAlignment);

        RegisterProperty<UI::FrameworkElement, UI::CornerRadius>(
            "CornerRadius",
            &UI::FrameworkElement::GetCornerRadius,
            &UI::FrameworkElement::SetCornerRadius);
    }
}
