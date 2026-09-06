#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <any>
#include <string>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling::Triggers {

class Setter : public Properties::DependencyObject {
public:
    using Properties::DependencyObject::SetValue;

    Setter() = default;
    Setter(const std::string& propertyName, const std::any& value);

    static const Properties::DependencyProperty& PropertyProperty();
    static const Properties::DependencyProperty& ValueProperty();

    void SetProperty(const std::string& propertyName) noexcept;
    const std::string& GetProperty() const noexcept;

    void SetValue(const std::any& value);
    const std::any& GetValue() const noexcept;

    void Apply(UIElement* element) const;

private:
    std::string m_propertyName;
    std::any m_value;
};

}
