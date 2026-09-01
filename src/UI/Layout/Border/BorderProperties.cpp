#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Markup/Xaml/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;
}

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeBorderPaddingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeBorderThicknessMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.PropertyChangedCallback = [](NeoWPF::UI::Properties::DependencyObject* sender,
                                              const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
            if (auto* border = dynamic_cast<NeoWPF::UI::Border*>(sender)) {
                border->SetBorderThickness(std::any_cast<NeoWPF::UI::Thickness>(args.NewValue));
            }
        };
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeBorderColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0x00000000));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeBorderBackgroundColorMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Color::FromArgb(0x00000000));
        metadata.Converter = NeoWPF::UI::Properties::GetColorConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }
}

namespace NeoWPF::UI::Properties {
    void RegisterBorderProperties() {
        RegisterProperty<UI::Border, UI::Thickness>("Padding",
            &UI::Border::GetPadding,
            static_cast<void (UI::Border::*)(UI::Thickness)>(&UI::Border::SetPadding),
            MakeBorderPaddingMetadata());
        RegisterProperty<UI::Border, UI::Thickness>("BorderThickness",
            MakeBorderThicknessMetadata());
        RegisterProperty<UI::Border, ::NeoWPF::UI::Color>("BorderBrush",
            &UI::Border::GetBorderColor,
            static_cast<void (UI::Border::*)(::NeoWPF::UI::Color)>(&UI::Border::SetBorderColor),
            MakeBorderColorMetadata());
        RegisterProperty<UI::Border, ::NeoWPF::UI::Color>("Background",
            &UI::Border::GetBackgroundColor,
            static_cast<void (UI::Border::*)(::NeoWPF::UI::Color)>(&UI::Border::SetBackgroundColor),
            MakeBorderBackgroundColorMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Border>("Border", "Content");
    }
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Border::PaddingProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Border, NeoWPF::UI::Thickness>(
        "Padding",
        &NeoWPF::UI::Border::GetPadding,
        static_cast<void (NeoWPF::UI::Border::*)(NeoWPF::UI::Thickness)>(&NeoWPF::UI::Border::SetPadding),
        MakeBorderPaddingMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Border::BorderThicknessProperty() {
    static const auto* property = &NeoWPF::UI::Properties::DependencyProperty::Register<
        NeoWPF::UI::Border, NeoWPF::UI::Thickness>("BorderThickness", MakeBorderThicknessMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Border::BorderColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Border, NeoWPF::UI::Color>(
        "BorderBrush",
        &NeoWPF::UI::Border::GetBorderColor,
        static_cast<void (NeoWPF::UI::Border::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Border::SetBorderColor),
        MakeBorderColorMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Border::BackgroundColorProperty() {
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::Border, NeoWPF::UI::Color>(
        "Background",
        &NeoWPF::UI::Border::GetBackgroundColor,
        static_cast<void (NeoWPF::UI::Border::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::Border::SetBackgroundColor),
        MakeBorderBackgroundColorMetadata());
    return *property;
}
