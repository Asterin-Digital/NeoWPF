#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

namespace NeoWPF::UI::Properties {

void RegisterProgressBarProperties()
{
    PropertyMetadata orientationMetadata;
    orientationMetadata.DefaultValue = std::any(static_cast<int>(UI::Orientation::Horizontal));
    orientationMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
    RegisterProperty<UI::ProgressBar, int>(
        "Orientation",
        static_cast<int (UI::ProgressBar::*)() const>(&UI::ProgressBar::GetOrientationValue),
        static_cast<void (UI::ProgressBar::*)(int)>(&UI::ProgressBar::SetOrientationValue),
        orientationMetadata);

    RegisterProperty<UI::ProgressBar, bool>(
        "IsIndeterminate",
        &UI::ProgressBar::GetIsIndeterminate,
        &UI::ProgressBar::SetIsIndeterminate,
        PropertyMetadata{std::any(false), {}, {}, PropertyFlags::AffectsRender, {}, GetBoolConverter()});

    RegisterProperty<UI::ProgressBar, float>(
        "AnimationProgress",
        &UI::ProgressBar::GetAnimationProgress,
        &UI::ProgressBar::SetAnimationProgress);
    RegisterProperty<UI::ProgressBar, float>(
        "IndeterminatePhase",
        &UI::ProgressBar::GetIndeterminatePhase,
        &UI::ProgressBar::SetIndeterminatePhase);

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::ProgressBar>(
        "ProgressBar", "", "NeoWPF::UI::ProgressBar", "NeoWPF/UI/Controls/ProgressBar/ProgressBar.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressBar::GetOrientationProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressBar, int>(
        "Orientation",
        static_cast<int (NeoWPF::UI::ProgressBar::*)() const>(&NeoWPF::UI::ProgressBar::GetOrientationValue),
        static_cast<void (NeoWPF::UI::ProgressBar::*)(int)>(&NeoWPF::UI::ProgressBar::SetOrientationValue));
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressBar::IsIndeterminateProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressBar, bool>(
        "IsIndeterminate", &NeoWPF::UI::ProgressBar::GetIsIndeterminate,
        &NeoWPF::UI::ProgressBar::SetIsIndeterminate);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressBar::AnimationProgressProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressBar, float>(
        "AnimationProgress", &NeoWPF::UI::ProgressBar::GetAnimationProgress,
        &NeoWPF::UI::ProgressBar::SetAnimationProgress);
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressBar::IndeterminatePhaseProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressBar, float>(
        "IndeterminatePhase", &NeoWPF::UI::ProgressBar::GetIndeterminatePhase,
        &NeoWPF::UI::ProgressBar::SetIndeterminatePhase);
    return property;
}
