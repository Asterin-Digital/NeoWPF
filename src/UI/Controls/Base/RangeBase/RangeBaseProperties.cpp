#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeRangeMetadata(
    double defaultValue,
    NeoWPF::UI::Properties::PropertyFlags flags = NeoWPF::UI::Properties::PropertyFlags::None)
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(defaultValue);
    metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
    metadata.Flags = flags;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterRangeBaseProperties()
    {
        PropertyMetadata minimumMetadata;
        minimumMetadata.DefaultValue = std::any(0.0);
        minimumMetadata.Converter = GetDoubleConverter();
        minimumMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::RangeBase, double>(
            "Minimum",
            &UI::RangeBase::GetMinimum,
            &UI::RangeBase::SetMinimum,
            minimumMetadata);

        PropertyMetadata maximumMetadata;
        maximumMetadata.DefaultValue = std::any(100.0);
        maximumMetadata.Converter = GetDoubleConverter();
        maximumMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsRender;
        RegisterProperty<UI::RangeBase, double>(
            "Maximum",
            &UI::RangeBase::GetMaximum,
            &UI::RangeBase::SetMaximum,
            maximumMetadata);

        PropertyMetadata valueMetadata;
        valueMetadata.DefaultValue = std::any(0.0);
        valueMetadata.Converter = GetDoubleConverter();
        valueMetadata.Flags = PropertyFlags::AffectsRender;
        RegisterProperty<UI::RangeBase, double>(
            "Value",
            &UI::RangeBase::GetValue,
            &UI::RangeBase::SetValue,
            valueMetadata);

        PropertyMetadata smallChangeMetadata;
        smallChangeMetadata.DefaultValue = std::any(1.0);
        smallChangeMetadata.Converter = GetDoubleConverter();
        RegisterProperty<UI::RangeBase, double>(
            "SmallChange",
            &UI::RangeBase::GetSmallChange,
            &UI::RangeBase::SetSmallChange,
            smallChangeMetadata);

        PropertyMetadata largeChangeMetadata;
        largeChangeMetadata.DefaultValue = std::any(10.0);
        largeChangeMetadata.Converter = GetDoubleConverter();
        RegisterProperty<UI::RangeBase, double>(
            "LargeChange",
            &UI::RangeBase::GetLargeChange,
            &UI::RangeBase::SetLargeChange,
            largeChangeMetadata);
}
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RangeBase::MinimumProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RangeBase, double>(
        "Minimum", &NeoWPF::UI::RangeBase::GetMinimum, &NeoWPF::UI::RangeBase::SetMinimum,
        MakeRangeMetadata(0.0, NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure |
                       NeoWPF::UI::Properties::PropertyFlags::AffectsRender));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RangeBase::MaximumProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RangeBase, double>(
        "Maximum", &NeoWPF::UI::RangeBase::GetMaximum, &NeoWPF::UI::RangeBase::SetMaximum,
        MakeRangeMetadata(100.0, NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure |
                       NeoWPF::UI::Properties::PropertyFlags::AffectsRender));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RangeBase::ValueProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RangeBase, double>(
        "Value", &NeoWPF::UI::RangeBase::GetValue, &NeoWPF::UI::RangeBase::SetValue,
        MakeRangeMetadata(0.0, NeoWPF::UI::Properties::PropertyFlags::AffectsRender));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RangeBase::SmallChangeProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RangeBase, double>(
        "SmallChange", &NeoWPF::UI::RangeBase::GetSmallChange, &NeoWPF::UI::RangeBase::SetSmallChange,
        MakeRangeMetadata(1.0));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::RangeBase::LargeChangeProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<NeoWPF::UI::RangeBase, double>(
        "LargeChange", &NeoWPF::UI::RangeBase::GetLargeChange, &NeoWPF::UI::RangeBase::SetLargeChange,
        MakeRangeMetadata(10.0));
    return *property;
}

void NeoWPF::UI::RangeBase::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    Control::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);

    const auto rawValue = GetRawValue(ValueProperty());
    if ((property.GetName() == "Minimum" || property.GetName() == "Maximum" || property.GetName() == "Value") &&
        rawValue.has_value() && rawValue.type() == typeid(double) &&
        m_value != std::any_cast<double>(rawValue)) {
        DependencyObject::SetValue(ValueProperty(), m_value);
    }
}
