#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

namespace NeoWPF::UI::Styling {
namespace {

Properties::PropertyMetadata MakeTargetTypeMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::type_index(typeid(void));
    metadata.Converter = Properties::GetTypeIndexConverter();
    return metadata;
}

Properties::PropertyMetadata MakeSettersMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::vector<std::shared_ptr<Triggers::Setter>>{};
    return metadata;
}

}

const Properties::DependencyProperty& Style::TargetTypeProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<Style, std::type_index>(
        "TargetType", MakeTargetTypeMetadata());
    return property;
}

const Properties::DependencyProperty& Style::SettersProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<Style, std::vector<std::shared_ptr<Setter>>>(
        "Setters", MakeSettersMetadata());
    return property;
}

}