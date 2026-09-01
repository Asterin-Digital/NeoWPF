#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeControlPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBackgroundColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBorderColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0xFFABADB3u));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBorderThicknessMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0);
        metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {

void RegisterControlProperties() {
    RegisterProperty<UI::Control, UI::Thickness>("Padding",
        &UI::Control::GetPadding,
        static_cast<void (UI::Control::*)(UI::Thickness)>(&UI::Control::SetPadding),
        MakeControlPaddingMetadata());
    RegisterProperty<UI::Control, ::NeoWPF::UI::Color>("Background",
        &UI::Control::GetBackgroundColor,
        static_cast<void (UI::Control::*)(::NeoWPF::UI::Color)>(&UI::Control::SetBackgroundColor),
        MakeControlBackgroundColorMetadata());
    RegisterProperty<UI::Control, ::NeoWPF::UI::Color>("BorderBrush",
        &UI::Control::GetBorderColor,
        static_cast<void (UI::Control::*)(::NeoWPF::UI::Color)>(&UI::Control::SetBorderColor),
        MakeControlBorderColorMetadata());
    RegisterProperty<UI::Control, int>("BorderThickness",
        &UI::Control::GetBorderThickness,
        &UI::Control::SetBorderThickness,
        MakeControlBorderThicknessMetadata());
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::PaddingProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, NeoWPF::UI::Thickness>(
        "Padding",
        &NeoWPF::UI::Control::GetPadding,
        static_cast<void (NeoWPF::UI::Control::*)(NeoWPF::UI::Thickness)>(&NeoWPF::UI::Control::SetPadding),
        MakeControlPaddingMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BackgroundColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, NeoWPF::UI::Color>(
        "Background",
        &NeoWPF::UI::Control::GetBackgroundColor,
        static_cast<void (NeoWPF::UI::Control::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Control::SetBackgroundColor),
        MakeControlBackgroundColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BorderColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, NeoWPF::UI::Color>(
        "BorderBrush",
        &NeoWPF::UI::Control::GetBorderColor,
        static_cast<void (NeoWPF::UI::Control::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Control::SetBorderColor),
        MakeControlBorderColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BorderThicknessProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, int>(
        "BorderThickness",
        &NeoWPF::UI::Control::GetBorderThickness,
        &NeoWPF::UI::Control::SetBorderThickness,
        MakeControlBorderThicknessMetadata());
    return *property;
}
