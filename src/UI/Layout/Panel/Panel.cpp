#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>
#include <algorithm>
#include <utility>

namespace NeoWPF::UI {

Panel::Panel() = default;

Panel::~Panel()
{
    m_lifetimeGuard->store(false, std::memory_order_release);
    const auto rules = GetResponsiveRules();
    for (const auto& rule : rules) {
        if (rule) {
            const auto subscription =
                m_responsiveRuleSubscriptions.find(rule.get());
            if (subscription != m_responsiveRuleSubscriptions.end()) {
                rule->RemoveChangedHandler(subscription->second);
            }
        }
    }
}

std::vector<std::shared_ptr<ResponsiveRule>>
Panel::GetResponsiveRules() const
{
    return GetValue<std::vector<std::shared_ptr<ResponsiveRule>>>(
        ResponsiveRulesProperty());
}

void Panel::AddResponsiveRule(std::shared_ptr<ResponsiveRule> rule)
{
    if (!rule) {
        return;
    }

    const auto& rules = GetResponsiveRules();
    if (std::find(rules.begin(), rules.end(), rule) != rules.end()) {
        return;
    }

    auto updatedRules = rules;
    updatedRules.push_back(rule);
    SetValue(ResponsiveRulesProperty(), updatedRules);
    InvalidateMeasure();
}

void Panel::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    Control::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);
    if (&property != &ResponsiveRulesProperty()) {
        return;
    }

    std::vector<std::shared_ptr<ResponsiveRule>> oldRules;
    std::vector<std::shared_ptr<ResponsiveRule>> newRules;
    if (oldValue.type() == typeid(std::vector<std::shared_ptr<ResponsiveRule>>)) {
        oldRules = std::any_cast<std::vector<std::shared_ptr<ResponsiveRule>>>(oldValue);
    }
    if (newValue.type() == typeid(std::vector<std::shared_ptr<ResponsiveRule>>)) {
        newRules = std::any_cast<std::vector<std::shared_ptr<ResponsiveRule>>>(newValue);
    }

    for (const auto& rule : oldRules) {
        if (!rule) {
            continue;
        }
        if (std::find(newRules.begin(), newRules.end(), rule) != newRules.end()) {
            continue;
        }
        const auto subscription = m_responsiveRuleSubscriptions.find(rule.get());
        if (subscription != m_responsiveRuleSubscriptions.end()) {
            rule->RemoveChangedHandler(subscription->second);
            m_responsiveRuleSubscriptions.erase(subscription);
        }
    }

    for (const auto& rule : newRules) {
        if (!rule ||
            m_responsiveRuleSubscriptions.find(rule.get()) !=
                m_responsiveRuleSubscriptions.end()) {
            continue;
        }
        auto lifetimeGuard = m_lifetimeGuard;
        const auto subscription = rule->AddChangedHandler([this, lifetimeGuard] {
            if (lifetimeGuard->load(std::memory_order_acquire)) {
                InvalidateMeasure();
            }
        });
        m_responsiveRuleSubscriptions.emplace(rule.get(), subscription);
    }
    InvalidateMeasure();
}

void Panel::EvaluateResponsiveRules(
    double availableWidth, double availableHeight)
{
    std::unordered_map<const Properties::DependencyProperty*, std::any>
        desiredValues;
    for (const auto& rule : GetResponsiveRules()) {
        if (!rule || !rule->Matches(availableWidth, availableHeight)) {
            continue;
        }

        for (const auto& setter : rule->GetSetters()) {
            if (!setter || setter->GetProperty().empty()) {
                continue;
            }

            const auto* const property =
                FindDependencyProperty(setter->GetProperty());
            if (!property) {
                NeoWPF::Diagnostics::LogError(
                    "ResponsiveRule references unknown dependency property '%s'",
                    setter->GetProperty().c_str());
                continue;
            }

            std::any value;
            if (!Properties::TryConvertValue(
                    *property, setter->GetValue(), value)) {
                NeoWPF::Diagnostics::LogError(
                    "ResponsiveRule could not convert value for dependency property '%s'",
                    setter->GetProperty().c_str());
                continue;
            }
            desiredValues[property] = std::move(value);
        }
    }

    for (auto value = m_activeResponsiveValues.begin();
         value != m_activeResponsiveValues.end();) {
        if (desiredValues.find(value->first) == desiredValues.end()) {
            ClearResponsiveValue(*value->first);
            value = m_activeResponsiveValues.erase(value);
        } else {
            ++value;
        }
    }

    for (auto& [property, value] : desiredValues) {
        const auto activeValue = m_activeResponsiveValues.find(property);
        if (activeValue != m_activeResponsiveValues.end() &&
            Properties::ValuesEqual(activeValue->second, value)) {
            continue;
        }

        SetValueInternal(
            *property, value, Properties::PropertyValueSource::Responsive);
        m_activeResponsiveValues[property] = std::move(value);
    }
}

void Panel::Measure(double availableWidth, double availableHeight)
{
    EvaluateResponsiveRules(availableWidth, availableHeight);

    double maxW = 0.0;
    double maxH = 0.0;
    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double childAvailableWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - margin.Horizontal()) : availableWidth;
            const double childAvailableHeight = availableHeight > 0.0 ? (std::max)(0.0, availableHeight - margin.Vertical()) : availableHeight;
            fe->Measure(childAvailableWidth, childAvailableHeight);
            const double resolvedW = fe->ResolveWidth(childAvailableWidth, fe->GetDesiredWidth()) + margin.Horizontal();
            const double resolvedH = fe->ResolveHeight(childAvailableHeight, fe->GetDesiredHeight()) + margin.Vertical();
            fe->SetDesiredSize((std::max)(0.0, fe->ResolveWidth(childAvailableWidth, fe->GetDesiredWidth())), (std::max)(0.0, fe->ResolveHeight(childAvailableHeight, fe->GetDesiredHeight())));
            maxW = (std::max)(maxW, resolvedW);
            maxH = (std::max)(maxH, resolvedH);
        }
    }

    const double desiredW = ResolveWidth(availableWidth, maxW);
    const double desiredH = ResolveHeight(availableHeight, maxH);

    SetDesiredSize(desiredW, desiredH);
}

void Panel::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    for (auto child : GetVisualChildren()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        if (auto fe = dynamic_cast<FrameworkElement*>(child)) {
            const auto margin = fe->GetMargin();
            const double contentWidth = (std::max)(0.0, finalWidth - margin.Horizontal());
            const double contentHeight = (std::max)(0.0, finalHeight - margin.Vertical());
            const double resolvedW = (std::max)(0.0, fe->ResolveWidth(contentWidth, fe->GetDesiredWidth()));
            const double resolvedH = (std::max)(0.0, fe->ResolveHeight(contentHeight, fe->GetDesiredHeight()));

            double alignedX = 0.0, alignedY = 0.0, alignedW = resolvedW, alignedH = resolvedH;
            Alignment::ApplyAlignment(
                fe,
                contentWidth, contentHeight,
                fe->GetHorizontalAlignment(),
                fe->GetVerticalAlignment(),
                alignedX, alignedY, alignedW, alignedH
            );

            if (fe->GetHorizontalAlignment() != HorizontalAlignment::Stretch) {
                alignedW = resolvedW;
            }
            if (fe->GetVerticalAlignment() != VerticalAlignment::Stretch) {
                alignedH = resolvedH;
            }

            fe->Arrange(alignedW, alignedH);
            fe->SetLayoutOffset(alignedX + margin.Left, alignedY + margin.Top);
        }
    }
}

void Panel::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    double absX = 0.0;
    double absY = 0.0;
    GetAbsolutePosition(absX, absY);

    if (const auto backgroundBrush = GetBackgroundBrush()) {
        const auto paint = ResolveBrushPaint(
            backgroundBrush, commands, absX, absY, GetFinalWidth(), GetFinalHeight());
        const auto cornerRadius = GetCornerRadius();
        if (!paint.IsEmpty()) {
            commands.DrawRoundedRect(
                absX,
                absY,
                GetFinalWidth(),
                GetFinalHeight(),
                cornerRadius.TopLeft,
                cornerRadius.TopRight,
                cornerRadius.BottomRight,
                cornerRadius.BottomLeft,
                paint,
                static_cast<float>(GetOpacity()));
        }
    }

    for (auto child : GetVisualChildrenInZOrder()) {
        if (auto uiChild = dynamic_cast<UIElement*>(child)) {
            if (!uiChild->IsVisible()) {
                continue;
            }
        }

        child->BuildRenderCommands(commands);
    }
}

}
