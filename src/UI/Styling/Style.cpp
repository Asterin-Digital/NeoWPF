#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

#include <any>
#include <memory>
#include <string>

namespace NeoWPF::UI::Styling {
namespace {

class TargetTypeConverter final : public Properties::TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override
    {
        const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(rawValue);
        if (!typeInfo) {
            return false;
        }
        convertedValue = typeInfo->Type;
        return true;
    }

    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override
    {
        const auto* typeInfo = NeoWPF::UI::Markup::Xaml::XamlTypeRegistry::Get().Find(rawValue);
        if (!typeInfo || typeInfo->CppTypeName.empty()) {
            expression.clear();
            return false;
        }
        expression = "std::type_index(typeid(" + typeInfo->CppTypeName + "))";
        return true;
    }
};

Properties::PropertyMetadata MakeTargetTypeMetadata()
{
    Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::type_index(typeid(void));
    metadata.Converter = std::make_shared<TargetTypeConverter>();
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

std::type_index Style::GetTargetType() const
{
    return GetValue<std::type_index>(TargetTypeProperty());
}

void Style::SetTargetType(const std::type_index& targetType)
{
    SetValue(TargetTypeProperty(), targetType);
}

const std::vector<std::shared_ptr<Triggers::Setter>>& Style::GetSetters() const noexcept
{
    return m_setters;
}

void Style::AddSetter(std::shared_ptr<Triggers::Setter> setter)
{
    if (!setter) {
        return;
    }
    m_setters.push_back(std::move(setter));
}

bool Style::ApplyTo(NeoWPF::UI::UIElement& element) const
{
    if (GetTargetType() != std::type_index(typeid(element))) {
        return false;
    }

    bool applied = false;
    for (const auto& setter : m_setters) {
        if (!setter || setter->GetProperty().empty()) {
            continue;
        }

        const auto* property = element.FindDependencyProperty(setter->GetProperty());
        if (!property) {
            continue;
        }

        std::any parsedValue;
        if (!Properties::TryConvertValue(*property, setter->GetValue(), parsedValue)) {
            continue;
        }

        if (element.SetStylePropertyValue(property->GetName(), parsedValue)) {
            applied = true;
        }
    }
    return applied;
}

void RegisterStyleProperties()
{
    Style::TargetTypeProperty();
    Style::SettersProperty();
    Triggers::Setter::PropertyProperty();
    Triggers::Setter::ValueProperty();
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<Style>(
        "Style", "Setters", "NeoWPF::UI::Styling::Style", "NeoWPF/UI/Styling/Style.h");
    NeoWPF::UI::Markup::Xaml::RegisterXamlType<Triggers::Setter>(
        "Setter", "", "NeoWPF::UI::Styling::Triggers::Setter", "NeoWPF/UI/Styling/Triggers/Setter.h");
    NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty<Style, Triggers::Setter>(
        Style::SettersProperty(), &Style::AddSetter, "AddSetter");
}

}

namespace NeoWPF::UI::Properties {

void RegisterStyleProperties()
{
    NeoWPF::UI::Styling::RegisterStyleProperties();
}

}
