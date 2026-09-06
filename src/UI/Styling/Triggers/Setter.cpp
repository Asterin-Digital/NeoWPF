#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>

#include <functional>

namespace {

using SetterValueCallback = std::function<void(
    NeoWPF::UI::Styling::Triggers::Setter&,
    const std::any&)>;

NeoWPF::UI::Properties::PropertyMetadata MakeSetterMetadata(SetterValueCallback callback)
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::string();
    metadata.Converter = NeoWPF::UI::Properties::GetStringConverter();
    metadata.PropertyChangedCallback = [callback = std::move(callback)](
        NeoWPF::UI::Properties::DependencyObject* sender,
        const NeoWPF::UI::Properties::DependencyPropertyChangedEventArgs& args) {
        if (auto* setter = dynamic_cast<NeoWPF::UI::Styling::Triggers::Setter*>(sender)) {
            callback(*setter, args.NewValue);
        }
    };
    return metadata;
}

std::string ReadSetterString(const std::any& value)
{
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(const char*)) {
        return std::any_cast<const char*>(value);
    }
    return {};
}

}

namespace NeoWPF::UI::Styling::Triggers {

const Properties::DependencyProperty& Setter::PropertyProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<Setter, std::string>(
        "Property", MakeSetterMetadata([](Setter& setter, const std::any& value) {
            setter.SetProperty(ReadSetterString(value));
        }));
    return property;
}

const Properties::DependencyProperty& Setter::ValueProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<Setter, std::string>(
        "Value", MakeSetterMetadata([](Setter& setter, const std::any& value) {
            setter.SetValue(ReadSetterString(value));
        }));
    return property;
}

Setter::Setter(const std::string& propertyName, const std::any& value)
    : m_propertyName(propertyName)
{
    SetValue(value);
}

void Setter::SetProperty(const std::string& propertyName) noexcept {
    m_propertyName = propertyName;
}

const std::string& Setter::GetProperty() const noexcept {
    return m_propertyName;
}

void Setter::SetValue(const std::any& value) {
    if (value.type() == typeid(const char*)) {
        m_value = std::string(std::any_cast<const char*>(value));
        return;
    }
    m_value = value;
}

const std::any& Setter::GetValue() const noexcept {
    return m_value;
}

void Setter::Apply(UIElement* element) const {
    if (!element || m_propertyName.empty()) {
        return;
    }

    const auto* property = element->FindDependencyProperty(m_propertyName);
    if (!property) {
        return;
    }

    element->SetTriggerPropertyValue(m_propertyName, m_value);
}

}
