#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeControlPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBackgroundBrushMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::shared_ptr<NeoWPF::UI::Brush>{});
        metadata.Converter = NeoWPF::UI::Properties::GetBrushConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBorderBrushMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::shared_ptr<NeoWPF::UI::Brush>{});
        metadata.Converter = NeoWPF::UI::Properties::GetBrushConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeControlBorderThicknessMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0.0);
        metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
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
    RegisterProperty<UI::Control, std::shared_ptr<UI::Brush>>("Background",
        &UI::Control::GetBackgroundBrush,
        &UI::Control::SetBackgroundBrush,
        MakeControlBackgroundBrushMetadata());
    RegisterProperty<UI::Control, std::shared_ptr<UI::Brush>>("BorderBrush",
        &UI::Control::GetBorderBrush,
        &UI::Control::SetBorderBrush,
        MakeControlBorderBrushMetadata());
    RegisterProperty<UI::Control, double>("BorderThickness",
        &UI::Control::GetBorderThickness,
        &UI::Control::SetBorderThickness,
        MakeControlBorderThicknessMetadata());
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::Control>(
        "Control", "", "NeoWPF::UI::Control", "NeoWPF/UI/Core/Element/Control.h");
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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BackgroundProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, std::shared_ptr<NeoWPF::UI::Brush>>(
        "Background",
        &NeoWPF::UI::Control::GetBackgroundBrush,
        &NeoWPF::UI::Control::SetBackgroundBrush,
        MakeControlBackgroundBrushMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BorderBrushProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, std::shared_ptr<NeoWPF::UI::Brush>>(
        "BorderBrush",
        &NeoWPF::UI::Control::GetBorderBrush,
        &NeoWPF::UI::Control::SetBorderBrush,
        MakeControlBorderBrushMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Control::BorderThicknessProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Control, double>(
        "BorderThickness",
        &NeoWPF::UI::Control::GetBorderThickness,
        &NeoWPF::UI::Control::SetBorderThickness,
        MakeControlBorderThicknessMetadata());
    return *property;
}
