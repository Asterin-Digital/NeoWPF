#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

PropertyMetadata MakeScrollBarBrushMetadata(std::shared_ptr<NeoWPF::UI::Brush> defaultBrush)
{
    PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::move(defaultBrush));
    metadata.Converter = GetBrushConverter();
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
        RegisterProperty<UI::ScrollBar, std::shared_ptr<UI::Brush>>(
            "TrackBrush",
            &UI::ScrollBar::GetTrackBrush,
            &UI::ScrollBar::SetTrackBrush,
            MakeScrollBarBrushMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
                NeoWPF::Core::ApplicationThemes::PaletteBrush::Track)));

        RegisterProperty<UI::ScrollBar, std::shared_ptr<UI::Brush>>(
            "ThumbBrush",
            &UI::ScrollBar::GetThumbBrush,
            &UI::ScrollBar::SetThumbBrush,
            MakeScrollBarBrushMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
                NeoWPF::Core::ApplicationThemes::PaletteBrush::Thumb)));

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

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::TrackBrushProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ScrollBar, std::shared_ptr<NeoWPF::UI::Brush>>(
            "TrackBrush",
            &NeoWPF::UI::ScrollBar::GetTrackBrush,
            &NeoWPF::UI::ScrollBar::SetTrackBrush,
            NeoWPF::UI::Properties::MakeScrollBarBrushMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
                NeoWPF::Core::ApplicationThemes::PaletteBrush::Track)));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ScrollBar::ThumbBrushProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::ScrollBar, std::shared_ptr<NeoWPF::UI::Brush>>(
            "ThumbBrush",
            &NeoWPF::UI::ScrollBar::GetThumbBrush,
            &NeoWPF::UI::ScrollBar::SetThumbBrush,
            NeoWPF::UI::Properties::MakeScrollBarBrushMetadata(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
                NeoWPF::Core::ApplicationThemes::PaletteBrush::Thumb)));
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
