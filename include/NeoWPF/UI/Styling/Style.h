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

/*
    @summary
    Groups setters that can be applied to a UI element.
*/
class Style final : public Properties::DependencyObject {
public:
    static const Properties::DependencyProperty& TargetTypeProperty();
    static const Properties::DependencyProperty& SettersProperty();

    std::type_index GetTargetType() const;
    /*
        @summary
        Sets the type targeted by the style.

        @param targetType
        Target UI type.
    */
    void SetTargetType(const std::type_index& targetType);

    const std::vector<std::shared_ptr<Setter>>& GetSetters() const noexcept;
    /*
        @summary
        Adds a setter to the style.

        @param setter
        Setter to add.
    */
    void AddSetter(std::shared_ptr<Triggers::Setter> setter);

    /*
        @summary
        Applies the style to an element.

        @param element
        Element to update.

        @returns
        True when the style targets the element type.
    */
    bool ApplyTo(NeoWPF::UI::UIElement& element) const;

private:
    std::vector<std::shared_ptr<Triggers::Setter>> m_setters;
};

void RegisterStyleProperties();

}
