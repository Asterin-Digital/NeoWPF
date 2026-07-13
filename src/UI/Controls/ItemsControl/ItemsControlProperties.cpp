#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterItemsControlProperties() {
        RegisterProperty<UI::ItemsControl, int>(
            "ItemSpacing",
            &UI::ItemsControl::GetItemSpacing,
            &UI::ItemsControl::SetItemSpacing);

        RegisterProperty<UI::ItemsControl, int>(
            "Padding",
            &UI::ItemsControl::GetPadding,
            &UI::ItemsControl::SetPadding);
    }
}
