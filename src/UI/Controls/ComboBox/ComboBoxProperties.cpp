#include <NeoWPF/UI/Controls/ComboBox/ComboBox.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeBoolMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(false);
    metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakePlacementMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(NeoWPF::UI::PopupPlacement::Bottom);
    metadata.Converter = NeoWPF::UI::Properties::GetPopupPlacementConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeSelectionBoxItemMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any{};
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeHighlightedMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(false);
    metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterComboBoxProperties()
{
    RegisterProperty<UI::ComboBox, bool>(
        "IsDropDownOpen",
        &UI::ComboBox::GetIsDropDownOpen,
        &UI::ComboBox::SetIsDropDownOpen,
        MakeBoolMetadata());
    RegisterProperty<UI::ComboBox, UI::PopupPlacement>(
        "Placement",
        &UI::ComboBox::GetPlacement,
        &UI::ComboBox::SetPlacement,
        MakePlacementMetadata());
    RegisterProperty<UI::ComboBox, std::any>(
        "SelectionBoxItem",
        &UI::ComboBox::GetSelectionBoxItem,
        nullptr,
        MakeSelectionBoxItemMetadata());
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::ComboBox>(
        "ComboBox", "Items", "NeoWPF::UI::ComboBox", "NeoWPF/UI/Controls/ComboBox/ComboBox.h");
}

void RegisterComboBoxItemProperties()
{
    RegisterProperty<UI::ComboBoxItem, bool>(
        "IsHighlighted",
        &UI::ComboBoxItem::GetIsHighlighted,
        &UI::ComboBoxItem::SetIsHighlighted,
        MakeHighlightedMetadata());
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::ComboBoxItem>(
        "ComboBoxItem", "Content", "NeoWPF::UI::ComboBoxItem", "NeoWPF/UI/Controls/ComboBox/ComboBox.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ComboBox::IsDropDownOpenProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ComboBox, bool>(
            "IsDropDownOpen",
            &NeoWPF::UI::ComboBox::GetIsDropDownOpen,
            &NeoWPF::UI::ComboBox::SetIsDropDownOpen,
            MakeBoolMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ComboBox::PlacementProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ComboBox, NeoWPF::UI::PopupPlacement>(
            "Placement",
            &NeoWPF::UI::ComboBox::GetPlacement,
            &NeoWPF::UI::ComboBox::SetPlacement,
            MakePlacementMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ComboBox::SelectionBoxItemProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ComboBox, std::any>(
            "SelectionBoxItem",
            &NeoWPF::UI::ComboBox::GetSelectionBoxItem,
            nullptr,
            MakeSelectionBoxItemMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ComboBoxItem::IsHighlightedProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ComboBoxItem, bool>(
            "IsHighlighted",
            &NeoWPF::UI::ComboBoxItem::GetIsHighlighted,
            &NeoWPF::UI::ComboBoxItem::SetIsHighlighted,
            MakeHighlightedMetadata());
    return *property;
}
