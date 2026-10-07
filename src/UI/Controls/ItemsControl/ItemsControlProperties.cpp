#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterItemsControlProperties() {
        const auto& itemsProperty = NeoWPF::UI::ItemsControl::ItemsProperty();
        NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty<
            UI::ItemsControl, UI::FrameworkElement>(itemsProperty,
                &UI::ItemsControl::AddItem,
                "AddItem");

        NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::ItemsControl>(
            "ItemsControl",
            "",
            "NeoWPF::UI::ItemsControl",
            "NeoWPF/UI/Controls/ItemsControl/ItemsControl.h");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ItemsControl::ItemsProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<
        NeoWPF::UI::ItemsControl, ItemCollection>(
            "Items",
            &NeoWPF::UI::ItemsControl::GetItems,
            &NeoWPF::UI::ItemsControl::SetItemsValue,
            [] {
                NeoWPF::UI::Properties::PropertyMetadata metadata;
                metadata.DefaultValue = std::any(ItemCollection{});
                return metadata;
            }());
    return property;
}
