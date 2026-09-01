#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterScrollBarProperties() {
        RegisterProperty<UI::ScrollBar, ::NeoWPF::UI::Color>(
            "TrackColor",
            &UI::ScrollBar::GetTrackColor,
            static_cast<void (UI::ScrollBar::*)(::NeoWPF::UI::Color)>(&UI::ScrollBar::SetTrackColor));

        RegisterProperty<UI::ScrollBar, ::NeoWPF::UI::Color>(
            "ThumbColor",
            &UI::ScrollBar::GetThumbColor,
            static_cast<void (UI::ScrollBar::*)(::NeoWPF::UI::Color)>(&UI::ScrollBar::SetThumbColor));

        RegisterProperty<UI::ScrollBar, int>(
            "ThumbSize",
            &UI::ScrollBar::GetThumbSize,
            &UI::ScrollBar::SetThumbSize);

        PropertyMetadata orientationMetadata;
        orientationMetadata.DefaultValue = std::any(static_cast<int>(UI::Orientation::Vertical));
        orientationMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::ScrollBar, int>(
            "Orientation",
            static_cast<int (UI::ScrollBar::*)() const>(&UI::ScrollBar::GetOrientationValue),
            static_cast<void (UI::ScrollBar::*)(int)>(&UI::ScrollBar::SetOrientationValue),
            orientationMetadata);
    }
}
