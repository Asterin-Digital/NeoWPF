#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterBorderProperties() {
        RegisterProperty<UI::Border, int>(
            "Padding",
            &UI::Border::GetPadding,
            &UI::Border::SetPadding);

        RegisterProperty<UI::Border, int>(
            "BorderThickness",
            &UI::Border::GetBorderThickness,
            &UI::Border::SetBorderThickness);

        RegisterProperty<UI::Border, NeoWPF::UI::Color>(
            "BorderColor",
            &UI::Border::GetBorderColor,
            static_cast<void (UI::Border::*)(NeoWPF::UI::Color)>(&UI::Border::SetBorderColor));

        RegisterProperty<UI::Border, NeoWPF::UI::Color>(
            "BackgroundColor",
            &UI::Border::GetBackgroundColor,
            static_cast<void (UI::Border::*)(NeoWPF::UI::Color)>(&UI::Border::SetBackgroundColor));
    }
}
