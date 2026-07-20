#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterControlProperties() {
    PropertyMetadata paddingMetadata;
    paddingMetadata.DefaultValue = std::any(0);
    paddingMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Control, int>(
        "Padding",
        &UI::Control::GetPadding,
        &UI::Control::SetPadding,
        paddingMetadata);

    PropertyMetadata backgroundMetadata;
    backgroundMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0x00000000u));
    backgroundMetadata.Flags = PropertyFlags::AffectsRender;
    RegisterProperty<UI::Control, NeoWPF::UI::Color>(
        "BackgroundColor",
        &UI::Control::GetBackgroundColor,
        static_cast<void (UI::Control::*)(NeoWPF::UI::Color)>(&UI::Control::SetBackgroundColor),
        backgroundMetadata);

    PropertyMetadata borderColorMetadata;
    borderColorMetadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0x00000000u));
    borderColorMetadata.Flags = PropertyFlags::AffectsRender;
    RegisterProperty<UI::Control, NeoWPF::UI::Color>(
        "BorderColor",
        &UI::Control::GetBorderColor,
        static_cast<void (UI::Control::*)(NeoWPF::UI::Color)>(&UI::Control::SetBorderColor),
        borderColorMetadata);

    PropertyMetadata borderThicknessMetadata;
    borderThicknessMetadata.DefaultValue = std::any(0);
    borderThicknessMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Control, int>(
        "BorderThickness",
        &UI::Control::GetBorderThickness,
        &UI::Control::SetBorderThickness,
        borderThicknessMetadata);
}

}
