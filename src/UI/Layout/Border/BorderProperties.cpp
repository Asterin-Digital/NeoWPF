#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterBorderProperties() {
        RegisterProperty<UI::Border, UI::Thickness>(
            "Padding",
            &UI::Border::GetPadding,
            [](UI::Border* owner, const UI::Thickness& value) {
                if (owner) {
                    owner->SetPadding(value);
                }
            });

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
