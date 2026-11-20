/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Layout/Panel/ResponsiveRule.h>
#include <NeoWPF/Rendering/Renderer.h>

#include <any>
#include <atomic>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI {

class Panel : public Control {
public:
    Panel();
    ~Panel() override;

    /*
        @summary
        Returns the dependency property for responsive rules.
    */
    static const Properties::DependencyProperty& ResponsiveRulesProperty();
    /*
        @summary
        Returns the responsive rules in evaluation order.
    */
    std::vector<std::shared_ptr<ResponsiveRule>> GetResponsiveRules() const;
    /*
        @summary
        Adds a responsive rule to the panel.

        @param rule
        Rule to evaluate against the panel's available size.
    */
    void AddResponsiveRule(std::shared_ptr<ResponsiveRule> rule);

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

protected:
    /*
        @summary
        Updates the responsive property layer for the current measure constraints.

        @param availableWidth
        Available width in device-independent units.

        @param availableHeight
        Available height in device-independent units.

        @internal
    */
    void EvaluateResponsiveRules(double availableWidth, double availableHeight);

    /*
        @internal
    */
    void OnDependencyPropertyChanged(
        const Properties::DependencyProperty& property,
        const Properties::PropertyMetadata& metadata,
        const std::any& oldValue,
        const std::any& newValue,
        Properties::PropertyValueSource source) override;

private:
    std::shared_ptr<std::atomic<bool>> m_lifetimeGuard =
        std::make_shared<std::atomic<bool>>(true);
    std::unordered_map<const ResponsiveRule*, std::size_t>
        m_responsiveRuleSubscriptions;
    std::unordered_map<const Properties::DependencyProperty*, std::any>
        m_activeResponsiveValues;
};

}
