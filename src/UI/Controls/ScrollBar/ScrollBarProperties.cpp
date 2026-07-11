#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterScrollBarProperties() {
        RegisterProperty<UI::ScrollBar, NeoWPF::UI::Color>(
            "TrackColor",
            &UI::ScrollBar::GetTrackColor,
            static_cast<void (UI::ScrollBar::*)(NeoWPF::UI::Color)>(&UI::ScrollBar::SetTrackColor));

        RegisterProperty<UI::ScrollBar, NeoWPF::UI::Color>(
            "ThumbColor",
            &UI::ScrollBar::GetThumbColor,
            static_cast<void (UI::ScrollBar::*)(NeoWPF::UI::Color)>(&UI::ScrollBar::SetThumbColor));

        RegisterProperty<UI::ScrollBar, int>(
            "ThumbSize",
            &UI::ScrollBar::GetThumbSize,
            &UI::ScrollBar::SetThumbSize);
    }
}
