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

        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty(
            setter->GetProperty(), GetTargetType());
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
