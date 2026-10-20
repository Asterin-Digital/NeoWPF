#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace NeoWPF::UI::Properties {
    void RegisterPanelProperties() {
        PropertyMetadata orientationMetadata;
        orientationMetadata.DefaultValue = std::any(static_cast<int>(UI::Orientation::Vertical));
        orientationMetadata.Converter = GetOrientationConverter();
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

