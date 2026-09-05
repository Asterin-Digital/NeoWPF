#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeUIElementOpacityMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterUIElementProperties() {
        RegisterProperty<UI::UIElement, bool>("Visibility", &UI::UIElement::IsVisible, &UI::UIElement::SetVisibility);
        RegisterProperty<UI::UIElement, bool>("IsEnabled", &UI::UIElement::IsEnabled, &UI::UIElement::SetEnabled);
        RegisterProperty<UI::UIElement, bool>("IsTabStop", &UI::UIElement::IsTabStop, &UI::UIElement::SetTabStop);
        RegisterProperty<UI::UIElement, int>("TabIndex", &UI::UIElement::GetTabIndex, &UI::UIElement::SetTabIndex);
        RegisterProperty<UI::UIElement, bool>("IsFocused", &UI::UIElement::IsFocused, &UI::UIElement::SetFocused);
        RegisterProperty<UI::UIElement, float>("Opacity", &UI::UIElement::GetOpacity, &UI::UIElement::SetOpacity, MakeUIElementOpacityMetadata());
        NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::UIElement>(
            "UIElement", "", "NeoWPF::UI::UIElement", "NeoWPF/UI/Core/Element/UIElement.h");
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             Visibility,
                             bool,
                             &NeoWPF::UI::UIElement::IsVisible,
                             &NeoWPF::UI::UIElement::SetVisibility,
                             []() { NeoWPF::UI::Properties::PropertyMetadata metadata; return metadata; })
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             IsEnabled,
                             bool,
                             &NeoWPF::UI::UIElement::IsEnabled,
                             &NeoWPF::UI::UIElement::SetEnabled,
                             []() { NeoWPF::UI::Properties::PropertyMetadata metadata; return metadata; })
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             IsTabStop,
                             bool,
                             &NeoWPF::UI::UIElement::IsTabStop,
                             &NeoWPF::UI::UIElement::SetTabStop,
                             []() { NeoWPF::UI::Properties::PropertyMetadata metadata; return metadata; })
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             TabIndex,
                             int,
                             &NeoWPF::UI::UIElement::GetTabIndex,
                             &NeoWPF::UI::UIElement::SetTabIndex,
                             []() { NeoWPF::UI::Properties::PropertyMetadata metadata; return metadata; })
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             IsFocused,
                             bool,
                             &NeoWPF::UI::UIElement::IsFocused,
                             &NeoWPF::UI::UIElement::SetFocused,
                             []() { NeoWPF::UI::Properties::PropertyMetadata metadata; return metadata; })
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::UIElement,
                             Opacity,
                             float,
                             &NeoWPF::UI::UIElement::GetOpacity,
                             &NeoWPF::UI::UIElement::SetOpacity,
                             MakeUIElementOpacityMetadata)
