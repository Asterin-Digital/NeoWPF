#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

PropertyMetadata MakeScrollBarColorMetadata(NeoWPF::UI::Color defaultColor)
{
    PropertyMetadata metadata;
    metadata.DefaultValue = std::any(defaultColor);
    metadata.Converter = GetColorConverter();
    metadata.Flags = PropertyFlags::AffectsRender;
    return metadata;
}

PropertyMetadata MakeScrollBarThumbSizeMetadata()
{
    double defaultSize = 24.0;
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().TryGetMetric(
        NeoWPF::Core::ApplicationThemes::ThemeMetric::ScrollBarThumbSize,
        defaultSize);
    PropertyMetadata metadata;
    metadata.DefaultValue = std::any(defaultSize);
    metadata.Converter = GetDoubleConverter();
    metadata.Flags = PropertyFlags::AffectsRender;
    return metadata;
}

void RegisterScrollBarProperties() {
        RegisterProperty<UI::ScrollBar, ::NeoWPF::UI::Color>(
            "TrackColor",
            &UI::ScrollBar::GetTrackColor,
            static_cast<void (UI::ScrollBar::*)(::NeoWPF::UI::Color)>(&UI::ScrollBar::SetTrackColor),
            MakeScrollBarColorMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
                NeoWPF::Core::ApplicationThemes::PaletteColor::Track)));

        RegisterProperty<UI::ScrollBar, ::NeoWPF::UI::Color>(
            "ThumbColor",
            &UI::ScrollBar::GetThumbColor,
            static_cast<void (UI::ScrollBar::*)(::NeoWPF::UI::Color)>(&UI::ScrollBar::SetThumbColor),
            MakeScrollBarColorMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
                NeoWPF::Core::ApplicationThemes::PaletteColor::Thumb)));

        RegisterProperty<UI::ScrollBar, double>(
            "ThumbSize",
            &UI::ScrollBar::GetThumbSize,
            &UI::ScrollBar::SetThumbSize,
            MakeScrollBarThumbSizeMetadata());

        ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::ScrollBar>(
            "ScrollBar", "", "NeoWPF::UI::ScrollBar", "NeoWPF/UI/Controls/ScrollBar/ScrollBar.h");

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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::TrackColorProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ScrollBar, NeoWPF::UI::Color>(
            "TrackColor",
            &NeoWPF::UI::ScrollBar::GetTrackColor,
            static_cast<void (NeoWPF::UI::ScrollBar::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::ScrollBar::SetTrackColor),
            NeoWPF::UI::Properties::MakeScrollBarColorMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
                NeoWPF::Core::ApplicationThemes::PaletteColor::Track)));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::ThumbColorProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ScrollBar, NeoWPF::UI::Color>(
            "ThumbColor",
            &NeoWPF::UI::ScrollBar::GetThumbColor,
            static_cast<void (NeoWPF::UI::ScrollBar::*)(NeoWPF::UI::Color)>(&NeoWPF::UI::ScrollBar::SetThumbColor),
            NeoWPF::UI::Properties::MakeScrollBarColorMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
                NeoWPF::Core::ApplicationThemes::PaletteColor::Thumb)));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::ThumbSizeProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ScrollBar, double>(
            "ThumbSize",
            &NeoWPF::UI::ScrollBar::GetThumbSize,
            &NeoWPF::UI::ScrollBar::SetThumbSize,
            NeoWPF::UI::Properties::MakeScrollBarThumbSizeMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::GetOrientationProperty()
{
    static const auto* property = [] {
        NeoWPF::UI::Properties::RegisterScrollBarProperties();
        return NeoWPF::UI::Properties::DependencyPropertyRegistry::Get().FindProperty(
            "Orientation", typeid(NeoWPF::UI::ScrollBar));
    }();
    return *property;
}
