#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeSelectedIndexMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(-1);
    metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeSelectedItemMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any{};
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeSelectionModeMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(NeoWPF::UI::SelectionMode::Single);
    metadata.Converter = NeoWPF::UI::Properties::GetSelectionModeConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterSelectorProperties()
{
    RegisterProperty<UI::Selector, int>(
        "SelectedIndex",
        &UI::Selector::GetSelectedIndex,
        &UI::Selector::SetSelectedIndex,
        MakeSelectedIndexMetadata());

    RegisterProperty<UI::Selector, std::any>(
        "SelectedItem",
        &UI::Selector::GetSelectedItem,
        [](UI::Selector* owner, const std::any& value) { if (owner) owner->SetSelectedItem(value); },
        MakeSelectedItemMetadata());

    RegisterProperty<UI::Selector, UI::SelectionMode>(
        "SelectionMode",
        &UI::Selector::GetSelectionMode,
        &UI::Selector::SetSelectionMode,
        MakeSelectionModeMetadata());

    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::Selector>(
        "Selector", "", "NeoWPF::UI::Selector", "NeoWPF/UI/Controls/Selector/Selector.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Selector::SelectedIndexProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Selector, int>(
            "SelectedIndex",
            &NeoWPF::UI::Selector::GetSelectedIndex,
            &NeoWPF::UI::Selector::SetSelectedIndex,
            MakeSelectedIndexMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Selector::SelectedItemProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Selector, std::any>(
            "SelectedItem",
            &NeoWPF::UI::Selector::GetSelectedItem,
            &NeoWPF::UI::Selector::SetSelectedItem,
            MakeSelectedItemMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Selector::SelectionModeProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Selector, NeoWPF::UI::SelectionMode>(
            "SelectionMode",
            &NeoWPF::UI::Selector::GetSelectionMode,
            &NeoWPF::UI::Selector::SetSelectionMode,
            MakeSelectionModeMetadata());
    return *property;
}
