#include <NeoWPF/UI/Controls/ItemsControl/ItemsControl.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterItemsControlProperties() {
        PropertyMetadata paddingMetadata;
        paddingMetadata.DefaultValue = std::any(UI::Thickness{});
        paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
        RegisterProperty<UI::ItemsControl, UI::Thickness>(
            "Padding",
            [](UI::ItemsControl* owner) -> UI::Thickness { return UI::Thickness(owner->GetPadding()); },
            [](UI::ItemsControl* owner, const UI::Thickness& value) {
                if (owner) {
                    owner->SetPadding(value.Left);
                }
            },
            paddingMetadata);
    }
}
