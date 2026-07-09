#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterScrollBarProperties() {
        RegisterProperty<UI::ScrollBar, uint32_t>(
            "TrackColor",
            &UI::ScrollBar::GetTrackColor,
            &UI::ScrollBar::SetTrackColor);

        RegisterProperty<UI::ScrollBar, uint32_t>(
            "ThumbColor",
            &UI::ScrollBar::GetThumbColor,
            &UI::ScrollBar::SetThumbColor);

        RegisterProperty<UI::ScrollBar, int>(
            "ThumbSize",
            &UI::ScrollBar::GetThumbSize,
            &UI::ScrollBar::SetThumbSize);
    }
}
