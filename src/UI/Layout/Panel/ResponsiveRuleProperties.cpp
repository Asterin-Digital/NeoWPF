#include <NeoWPF/UI/Layout/Panel/ResponsiveRule.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

#include <limits>

namespace NeoWPF::UI {
namespace {

Properties::PropertyMetadata MakeBoundMetadata(double defaultValue)
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = defaultValue;
    metadata.Converter = Properties::GetDoubleConverter();
    return metadata;
}

Properties::PropertyMetadata MakeSettersMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue =
        std::vector<std::shared_ptr<Styling::Triggers::Setter>>{};
    return metadata;
}

}

const Properties::DependencyProperty& ResponsiveRule::MinWidthProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, double>(
            "MinWidth", MakeBoundMetadata(
                -std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MaxWidthProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, double>(
            "MaxWidth", MakeBoundMetadata(
                std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MinHeightProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, double>(
            "MinHeight", MakeBoundMetadata(
                -std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::MaxHeightProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<ResponsiveRule, double>(
            "MaxHeight", MakeBoundMetadata(
                std::numeric_limits<double>::infinity()));
    return property;
}

const Properties::DependencyProperty& ResponsiveRule::SettersProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<
            ResponsiveRule,
            std::vector<std::shared_ptr<Styling::Triggers::Setter>>>(
                "Setters", MakeSettersMetadata());
    return property;
}

}
