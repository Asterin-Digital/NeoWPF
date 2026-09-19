#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

namespace NeoWPF::UI::Properties {

void RegisterSliderProperties()
{
    RegisterProperty<UI::Slider, bool>("IsDirectionReversed",
        &UI::Slider::GetIsDirectionReversed, &UI::Slider::SetIsDirectionReversed,
        PropertyMetadata{std::any(false), {}, {}, PropertyFlags::AffectsRender, {}, GetBoolConverter()});
    RegisterProperty<UI::Slider, bool>("IsSnapToTickEnabled",
        &UI::Slider::GetIsSnapToTickEnabled, &UI::Slider::SetIsSnapToTickEnabled,
        PropertyMetadata{std::any(false), {}, {}, PropertyFlags::AffectsRender, {}, GetBoolConverter()});
    RegisterProperty<UI::Slider, double>("TickFrequency",
        &UI::Slider::GetTickFrequency, &UI::Slider::SetTickFrequency,
        PropertyMetadata{std::any(1.0), {}, {}, PropertyFlags::AffectsRender, {}, GetDoubleConverter()});
    RegisterProperty<UI::Slider, int>("TickPlacement",
        &UI::Slider::GetTickPlacementValue, &UI::Slider::SetTickPlacementValue);
    RegisterProperty<UI::Slider, bool>("IsSelectionRangeEnabled",
        &UI::Slider::GetIsSelectionRangeEnabled, &UI::Slider::SetIsSelectionRangeEnabled,
        PropertyMetadata{std::any(false), {}, {}, PropertyFlags::AffectsRender, {}, GetBoolConverter()});
    RegisterProperty<UI::Slider, double>("SelectionStart",
        &UI::Slider::GetSelectionStart, &UI::Slider::SetSelectionStart,
        PropertyMetadata{std::any(0.0), {}, {}, PropertyFlags::AffectsRender, {}, GetDoubleConverter()});
    RegisterProperty<UI::Slider, double>("SelectionEnd",
        &UI::Slider::GetSelectionEnd, &UI::Slider::SetSelectionEnd,
        PropertyMetadata{std::any(0.0), {}, {}, PropertyFlags::AffectsRender, {}, GetDoubleConverter()});

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Slider>(
        "Slider", "", "NeoWPF::UI::Slider", "NeoWPF/UI/Controls/Slider/Slider.h");

    PropertyMetadata orientationMetadata;
    orientationMetadata.DefaultValue = std::any(static_cast<int>(UI::Orientation::Horizontal));
    orientationMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::Slider, int>("Orientation",
        static_cast<int (UI::Slider::*)() const>(&UI::Slider::GetOrientationValue),
        static_cast<void (UI::Slider::*)(int)>(&UI::Slider::SetOrientationValue),
        orientationMetadata);
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::GetOrientationProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, int>(
        "Orientation",
        static_cast<int (NeoWPF::UI::Slider::*)() const>(&NeoWPF::UI::Slider::GetOrientationValue),
        static_cast<void (NeoWPF::UI::Slider::*)(int)>(&NeoWPF::UI::Slider::SetOrientationValue));
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::IsDirectionReversedProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, bool>(
        "IsDirectionReversed", &NeoWPF::UI::Slider::GetIsDirectionReversed, &NeoWPF::UI::Slider::SetIsDirectionReversed);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::IsSnapToTickEnabledProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, bool>(
        "IsSnapToTickEnabled", &NeoWPF::UI::Slider::GetIsSnapToTickEnabled, &NeoWPF::UI::Slider::SetIsSnapToTickEnabled);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::TickFrequencyProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, double>(
        "TickFrequency", &NeoWPF::UI::Slider::GetTickFrequency, &NeoWPF::UI::Slider::SetTickFrequency);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::TickPlacementProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, int>(
        "TickPlacement", &NeoWPF::UI::Slider::GetTickPlacementValue, &NeoWPF::UI::Slider::SetTickPlacementValue);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::TicksProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, NeoWPF::UI::Core::DoubleCollection>(
        "Ticks",
        nullptr,
        nullptr,
        {});
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::IsSelectionRangeEnabledProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, bool>(
        "IsSelectionRangeEnabled", &NeoWPF::UI::Slider::GetIsSelectionRangeEnabled, &NeoWPF::UI::Slider::SetIsSelectionRangeEnabled);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::SelectionStartProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, double>(
        "SelectionStart", &NeoWPF::UI::Slider::GetSelectionStart, &NeoWPF::UI::Slider::SetSelectionStart);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Slider::SelectionEndProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::Slider, double>(
        "SelectionEnd", &NeoWPF::UI::Slider::GetSelectionEnd, &NeoWPF::UI::Slider::SetSelectionEnd);
    return property;
}