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

        RegisterProperty<UI::Border, uint32_t>(
            "BorderColor",
            &UI::Border::GetBorderColor,
            &UI::Border::SetBorderColor);

        RegisterProperty<UI::Border, uint32_t>(
            "BackgroundColor",
            &UI::Border::GetBackgroundColor,
            &UI::Border::SetBackgroundColor);
    }
}
