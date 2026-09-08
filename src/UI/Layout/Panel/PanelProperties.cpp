#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakePanelBackgroundColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0x00000000));
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterPanelProperties() {
        RegisterProperty<UI::Panel, ::NeoWPF::UI::Color>("Background",
            &UI::Panel::GetBackgroundColor,
            static_cast<void (UI::Panel::*)(::NeoWPF::UI::Color)>(&UI::Panel::SetBackgroundColor),
            MakePanelBackgroundColorMetadata());
        PropertyMetadata orientationMetadata;
        orientationMetadata.DefaultValue = std::any(static_cast<int>(UI::Orientation::Vertical));
        orientationMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        orientationMetadata.PropertyChangedCallback = [](DependencyObject* sender, const DependencyPropertyChangedEventArgs& args) {
            if (auto* stackPanel = dynamic_cast<UI::StackPanel*>(sender)) {
                stackPanel->SetOrientation(static_cast<UI::Orientation>(std::any_cast<int>(args.NewValue)));
            }
        };
        RegisterProperty<UI::StackPanel, int>("Orientation", orientationMetadata);
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Panel>(
            "Panel", "Children", "NeoWPF::UI::Panel", "NeoWPF/UI/Layout/Panel/Panel.h");
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::StackPanel>(
            "StackPanel", "Children", "NeoWPF::UI::StackPanel", "NeoWPF/UI/Layout/Panel/StackPanel.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Panel::BackgroundColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Panel, NeoWPF::UI::Color>(
        "Background",
        &NeoWPF::UI::Panel::GetBackgroundColor,
        static_cast<void (NeoWPF::UI::Panel::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Panel::SetBackgroundColor),
        MakePanelBackgroundColorMetadata());
    return *property;
}
