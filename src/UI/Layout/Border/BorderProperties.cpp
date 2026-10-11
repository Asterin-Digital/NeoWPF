#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
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

}

namespace NeoWPF::UI::Properties {
    void RegisterBorderProperties() {
        RegisterProperty<UI::Border, UI::Thickness>("Padding",
            &UI::Border::GetPadding,
            static_cast<void (UI::Border::*)(UI::Thickness)>(&UI::Border::SetPadding),
            MakeBorderPaddingMetadata());
        RegisterProperty<UI::Border, UI::Thickness>("BorderThickness",
            MakeBorderThicknessMetadata());
        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Border>(
            "Border", "Content", "NeoWPF::UI::Border", "NeoWPF/UI/Layout/Border/Border.h");
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

