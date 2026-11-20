#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& Panel::ResponsiveRulesProperty()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::vector<std::shared_ptr<ResponsiveRule>>{};
    static const auto& property =
        Properties::DependencyProperty::Register<
            Panel, std::vector<std::shared_ptr<ResponsiveRule>>>(
                "ResponsiveRules", metadata);
    return property;
}

}

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
        UI::Panel::ResponsiveRulesProperty();
        UI::ResponsiveRule::MinWidthProperty();
        UI::ResponsiveRule::MaxWidthProperty();
        UI::ResponsiveRule::MinHeightProperty();
        UI::ResponsiveRule::MaxHeightProperty();
        UI::ResponsiveRule::SettersProperty();
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Panel>(
            "Panel", "Children", "NeoWPF::UI::Panel", "NeoWPF/UI/Layout/Panel/Panel.h");
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::StackPanel>(
            "StackPanel", "Children", "NeoWPF::UI::StackPanel", "NeoWPF/UI/Layout/Panel/StackPanel.h");
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::ResponsiveRule>(
            "ResponsiveRule", "Setters", "NeoWPF::UI::ResponsiveRule",
            "NeoWPF/UI/Layout/Panel/ResponsiveRule.h");
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty<
            ::NeoWPF::UI::Panel, ::NeoWPF::UI::ResponsiveRule>(
                UI::Panel::ResponsiveRulesProperty(),
                &UI::Panel::AddResponsiveRule, "AddResponsiveRule");
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty<
            ::NeoWPF::UI::ResponsiveRule, ::NeoWPF::UI::Styling::Triggers::Setter>(
                UI::ResponsiveRule::SettersProperty(),
                &UI::ResponsiveRule::AddSetter, "AddSetter");
    }
}

