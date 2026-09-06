#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <any>
#include <memory>
#include <typeindex>
#include <vector>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Styling {

namespace Triggers {
class Setter;
}

using Setter = Triggers::Setter;

class Style final : public Properties::DependencyObject {
public:
    static const Properties::DependencyProperty& TargetTypeProperty();
    static const Properties::DependencyProperty& SettersProperty();

    std::type_index GetTargetType() const;
    void SetTargetType(const std::type_index& targetType);

    const std::vector<std::shared_ptr<Setter>>& GetSetters() const noexcept;
    void AddSetter(std::shared_ptr<Triggers::Setter> setter);
    bool ApplyTo(NeoWPF::UI::UIElement& element) const;

private:
    std::vector<std::shared_ptr<Triggers::Setter>> m_setters;
};

void RegisterStyleProperties();

}
