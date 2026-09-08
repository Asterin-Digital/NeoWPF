#include <NeoWPF/UI/Controls/ListBox/ListBox.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeListBoxItemIsSelectedMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(false);
    metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterListBoxItemProperties()
{
    RegisterProperty<UI::ListBoxItem, bool>(
        "IsSelected",
        &UI::ListBoxItem::GetIsSelected,
        &UI::ListBoxItem::SetIsSelected,
        MakeListBoxItemIsSelectedMetadata());
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::ListBoxItem>(
        "ListBoxItem", "", "NeoWPF::UI::ListBoxItem", "NeoWPF/UI/Controls/ListBox/ListBox.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ListBoxItem::IsSelectedProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ListBoxItem, bool>(
            "IsSelected",
            &NeoWPF::UI::ListBoxItem::GetIsSelected,
            &NeoWPF::UI::ListBoxItem::SetIsSelected,
            MakeListBoxItemIsSelectedMetadata());
    return *property;
}
