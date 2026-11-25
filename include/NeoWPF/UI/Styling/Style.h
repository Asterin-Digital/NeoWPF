/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <any>
#include <memory>
#include <typeindex>
#include <vector>

namespace NeoWPF::UI {
class UIElement;
class ResponsiveRule;
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
    static const Properties::DependencyProperty& ResponsiveRulesProperty();

    /*
        @summary
        Returns the type targeted by the style.
    */
    std::type_index GetTargetType() const;
    /*
        @summary
        Sets the type targeted by the style.

        @param targetType
        Target UI type.
    */
    void SetTargetType(const std::type_index& targetType);

    /*
        @summary
        Returns the setters currently registered on the style.
    */
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
        Returns the responsive rules registered on the style.
    */
    const std::vector<std::shared_ptr<ResponsiveRule>>& GetResponsiveRules() const noexcept;

    /*
        @summary
        Adds a responsive rule to the style.

        @param rule
        Rule to host on each target element.
    */
    void AddResponsiveRule(std::shared_ptr<ResponsiveRule> rule);

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
    std::vector<std::shared_ptr<ResponsiveRule>> m_responsiveRules;
};

void RegisterStyleProperties();

}
