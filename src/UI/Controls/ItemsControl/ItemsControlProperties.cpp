#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterItemsControlProperties() {
        PropertyMetadata paddingMetadata;
        paddingMetadata.DefaultValue = std::any(0);
        paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        RegisterProperty<UI::ItemsControl, int>(
            "Padding",
            &UI::ItemsControl::GetPadding,
            &UI::ItemsControl::SetPadding,
            paddingMetadata);
    }
}
