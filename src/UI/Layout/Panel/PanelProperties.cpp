#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterPanelProperties() {
        RegisterProperty<UI::Panel, uint32_t>(
            "BackgroundColor",
            &UI::Panel::GetBackgroundColor,
            &UI::Panel::SetBackgroundColor);
    }
}
