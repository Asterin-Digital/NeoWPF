#include <NeoWPF/UI/Controls/ProgressRing/ProgressRing.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

namespace NeoWPF::UI::Properties {

void RegisterProgressRingProperties()
{
    RegisterProperty<UI::ProgressRing, bool>(
        "IsIndeterminate",
        &UI::ProgressRing::GetIsIndeterminate,
        &UI::ProgressRing::SetIsIndeterminate,
        PropertyMetadata{std::any(false), {}, {}, PropertyFlags::AffectsRender, {}, GetBoolConverter()});

    RegisterProperty<UI::ProgressRing, double>(
        "RingThickness",
        &UI::ProgressRing::GetRingThickness,
        &UI::ProgressRing::SetRingThickness,
        PropertyMetadata{std::any(6.0), {}, {}, PropertyFlags::AffectsRender, {}, GetDoubleConverter()});

    RegisterProperty<UI::ProgressRing, float>(
        "AnimationProgress",
        &UI::ProgressRing::GetAnimationProgress,
        &UI::ProgressRing::SetAnimationProgress,
        PropertyMetadata{std::any(0.0f), {}, {}, PropertyFlags::AffectsRender});

    RegisterProperty<UI::ProgressRing, float>(
        "IndeterminatePhase",
        &UI::ProgressRing::GetIndeterminatePhase,
        &UI::ProgressRing::SetIndeterminatePhase,
        PropertyMetadata{std::any(0.0f), {}, {}, PropertyFlags::AffectsRender});

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::ProgressRing>(
        "ProgressRing", "", "NeoWPF::UI::ProgressRing", "NeoWPF/UI/Controls/ProgressRing/ProgressRing.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressRing::IsIndeterminateProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressRing, bool>(
        "IsIndeterminate", &NeoWPF::UI::ProgressRing::GetIsIndeterminate,
        &NeoWPF::UI::ProgressRing::SetIsIndeterminate,
        NeoWPF::UI::Properties::PropertyMetadata{std::any(false), {}, {}, NeoWPF::UI::Properties::PropertyFlags::AffectsRender, {}, NeoWPF::UI::Properties::GetBoolConverter()});
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressRing::RingThicknessProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressRing, double>(
        "RingThickness", &NeoWPF::UI::ProgressRing::GetRingThickness,
        &NeoWPF::UI::ProgressRing::SetRingThickness,
        NeoWPF::UI::Properties::PropertyMetadata{std::any(6.0), {}, {}, NeoWPF::UI::Properties::PropertyFlags::AffectsRender, {}, NeoWPF::UI::Properties::GetDoubleConverter()});
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressRing::AnimationProgressProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressRing, float>(
        "AnimationProgress", &NeoWPF::UI::ProgressRing::GetAnimationProgress,
        &NeoWPF::UI::ProgressRing::SetAnimationProgress,
        NeoWPF::UI::Properties::PropertyMetadata{std::any(0.0f), {}, {}, NeoWPF::UI::Properties::PropertyFlags::AffectsRender});
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::ProgressRing::IndeterminatePhaseProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterProperty<NeoWPF::UI::ProgressRing, float>(
        "IndeterminatePhase", &NeoWPF::UI::ProgressRing::GetIndeterminatePhase,
        &NeoWPF::UI::ProgressRing::SetIndeterminatePhase,
        NeoWPF::UI::Properties::PropertyMetadata{std::any(0.0f), {}, {}, NeoWPF::UI::Properties::PropertyFlags::AffectsRender});
    return property;
}