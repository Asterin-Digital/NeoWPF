#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterPanelProperties() {
        RegisterProperty<UI::Panel, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::Panel::GetBackgroundColor,
            static_cast<void (UI::Panel::*)(NeoWPF::UI::Color)>(&UI::Panel::SetBackgroundColor));
    }
}
